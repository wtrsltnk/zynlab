/*
 * "$Id$"
 *
 * Documentation generator using Mini-XML, a small XML-like file parsing
 * library.
 *
 * Copyright 2003-2007 by Michael Sweet.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Contents:
 *
 *   main()                - Main entry for test program.
 *   add_variable()        - Add a variable or argument.
 *   get_comment_info()    - Get info from comment.
 *   get_text()            - Get the text for a node.
 *   load_cb()             - Set the type of child nodes.
 *   new_documentation()   - Create a new documentation tree.
 *   safe_strcpy()         - Copy a string allowing for overlapping strings.
 *   scan_file()           - Scan a source file.
 *   sort_node()           - Insert a node sorted into a tree.
 *   update_comment()      - Update a comment node.
 *   usage()               - Show program usage...
 *   write_description()   - Write the description text.
 *   write_element()       - Write an elements text nodes.
 *   write_html()          - Write HTML documentation.
 *   write_man()           - Write manpage documentation.
 *   write_string()        - Write a string, quoting XHTML special chars
 *                           as needed...
 *   ws_cb()               - Whitespace callback for saving.
 */

/*
 * Include necessary headers...
 */

#include "config.h"
#include "mxml.h"
#include <time.h>


/*
 * This program scans source and header files and produces public API
 * documentation for code that conforms to the CUPS Configuration
 * Management Plan (CMP) coding standards.  Please see the following web
 * page for details:
 *
 *     http://www.cups.org/cmp.html
 *
 * Using Mini-XML, this program creates and maintains an XML representation
 * of the public API code documentation which can then be converted to HTML
 * as desired.  The following is a poor-man's schema:
 *
 * <?xml version="1.0"?>
 * <mxmldoc xmlns="http://www.easysw.com"
 *  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
 *  xsi:schemaLocation="http://www.minixml.org/mxmldoc.xsd">
 *
 *   <namespace name="">                        [optional...]
 *     <constant name="">
 *       <description>descriptive text</description>
 *     </constant>
 *  
 *     <enumeration name="">
 *       <description>descriptive text</description>
 *       <constant name="">...</constant>
 *     </enumeration>
 *  
 *     <typedef name="">
 *       <description>descriptive text</description>
 *       <type>type string</type>
 *     </typedef>
 *  
 *     <function name="" scope="">
 *       <description>descriptive text</description>
 *       <argument name="" direction="I|O|IO" default="">
 *         <description>descriptive text</description>
 *         <type>type string</type>
 *       </argument>
 *       <returnvalue>
 *         <description>descriptive text</description>
 *         <type>type string</type>
 *       </returnvalue>
 *       <seealso>function names separated by spaces</seealso>
 *     </function>
 *  
 *     <variable name="" scope="">
 *       <description>descriptive text</description>
 *       <type>type string</type>
 *     </variable>
 *  
 *     <struct name="">
 *       <description>descriptive text</description>
 *       <variable name="">...</variable>
 *       <function name="">...</function>
 *     </struct>
 *  
 *     <union name="">
 *       <description>descriptive text</description>
 *       <variable name="">...</variable>
 *     </union>
 *  
 *     <class name="" parent="">
 *       <description>descriptive text</description>
 *       <class name="">...</class>
 *       <enumeration name="">...</enumeration>
 *       <function name="">...</function>
 *       <struct name="">...</struct>
 *       <variable name="">...</variable>
 *     </class>
 *   </namespace>
 * </mxmldoc>
 */
 

/*
 * Basic states for file parser...
 */

#define STATE_NONE		0	/* No state - whitespace, etc. */
#define STATE_PREPROCESSOR	1	/* Preprocessor directive */
#define STATE_C_COMMENT		2	/* Inside a C comment */
#define STATE_CXX_COMMENT	3	/* Inside a C++ comment */
#define STATE_STRING		4	/* Inside a string constant */
#define STATE_CHARACTER		5	/* Inside a character constant */
#define STATE_IDENTIFIER	6	/* Inside a keyword/identifier */


/*
 * Output modes...
 */

#define OUTPUT_NONE		0	/* No output */
#define OUTPUT_HTML		1	/* Output HTML */
#define OUTPUT_MAN		2	/* Output nroff/man */


/*
 * Local functions...
 */

static mxml_node_t	*add_variable(mxml_node_t *parent, const char *name,
			              mxml_node_t *type);
static mxml_node_t	*find_public(mxml_node_t *node, mxml_node_t *top,
			             const char *name);
static char		*get_comment_info(mxml_node_t *description);
static char		*get_text(mxml_node_t *node, char *buffer, int buflen);
static mxml_type_t	load_cb(mxml_node_t *node);
static mxml_node_t	*new_documentation(mxml_node_t **mxmldoc);
static void		safe_strcpy(char *dst, const char *src);
static int		scan_file(const char *filename, FILE *fp,
			          mxml_node_t *doc);
static void		sort_node(mxml_node_t *tree, mxml_node_t *func);
static void		update_comment(mxml_node_t *parent,
			               mxml_node_t *comment);
static void		usage(const char *option);
static void		write_description(mxml_node_t *description, int mode);
static void		write_element(mxml_node_t *doc, mxml_node_t *element,
			              int mode);
static void		write_html(const char *section,
			           const char *title,
			           const char *intro,
			           mxml_node_t *doc);
static void		write_man(const char *man_name,
			          const char *section,
			          const char *title,
			          const char *intro,
			          mxml_node_t *doc);
static void		write_string(const char *s, int mode);
static const char	*ws_cb(mxml_node_t *node, int where);


/*
 * 'main()' - Main entry for test program.
 */

int					/* O - Exit status */
main(int  argc,				/* I - Number of command-line args */
     char *argv[])			/* I - Command-line args */
{
  int		i;			/* Looping var */
  int		len;			/* Length of argument */
  FILE		*fp;			/* File to read */
  mxml_node_t	*doc;			/* XML documentation tree */
  mxml_node_t	*mxmldoc;		/* mxmldoc node */
  const char	*section;		/* Section/keywords of documentation */
  const char	*title;			/* Title of documentation */
  const char	*introfile;		/* Introduction file */
  const char	*xmlfile;		/* XML file */
  const char	*name;			/* Name of manpage */
  int		update;			/* Updated XML file */
  int		mode;			/* Output mode */

 /*
  * Check arguments...
  */

  name      = NULL;
  section   = NULL;
  title     = NULL;
  introfile = NULL;
  xmlfile   = NULL;
  update    = 0;
  doc       = NULL;
  mxmldoc   = NULL;
  mode      = OUTPUT_HTML;

  for (i = 1; i < argc; i ++)
    if (!strcmp(argv[i], "--help"))
    {
     /*
      * Show help...
      */

      usage(NULL);
    }
    else if (!strcmp(argv[i], "--intro") && !introfile)
    {
     /*
      * Set intro file...
      */

      i ++;
      if (i < argc)
        introfile = argv[i];
      else
        usage(NULL);
    }
    else if (!strcmp(argv[i], "--man") && !name)
    {
     /*
      * Output manpage...
      */

      i ++;
      if (i < argc)
      {
        mode = OUTPUT_MAN;
        name = argv[i];
      }
      else
        usage(NULL);
    }
    else if (!strcmp(argv[i], "--no-output"))
      mode = OUTPUT_NONE;
    else if (!strcmp(argv[i], "--section") && !section)
    {
     /*
      * Set section/keywords...
      */

      i ++;
      if (i < argc)
        section = argv[i];
      else
        usage(NULL);
    }
    else if (!strcmp(argv[i], "--title") && !title)
    {
     /*
      * Set title...
      */

      i ++;
      if (i < argc)
        title = argv[i];
      else
        usage(NULL);
    }
    else if (argv[i][0] == '-')
    {
     /*
      * Unknown/bad option...
      */

      usage(argv[i]);
    }
    else
    {
     /*
      * Process XML or source file...
      */

      len = (int)strlen(argv[i]);
      if (len > 4 && !strcmp(argv[i] + len - 4, ".xml"))
      {
       /*
        * Set XML file...
	*/

        if (xmlfile)
	  usage(NULL);

        xmlfile = argv[i];

        if (!doc)
	{
	  if ((fp = fopen(argv[i], "r")) != NULL)
	  {
	   /*
	    * Read the existing XML file...
	    */

	    doc = mxmlLoadFile(NULL, fp, load_cb);

	    fclose(fp);

	    if (!doc)
	    {
	      mxmldoc = NULL;

	      fprintf(stderr, "mxmldoc: Unable to read the XML documentation file \"%s\"!\n",
        	      argv[i]);
	    }
	    else if ((mxmldoc = mxmlFindElement(doc, doc, "mxmldoc", NULL,
                                        	NULL, MXML_DESCEND)) == NULL)
	    {
	      fprintf(stderr, "mxmldoc: XML documentation file \"%s\" is missing <mxmldoc> node!!\n",
        	      argv[i]);

	      mxmlDelete(doc);
	      doc = NULL;
	    }
	  }
	  else
	  {
	    doc     = NULL;
	    mxmldoc = NULL;
	  }

	  if (!doc)
	    doc = new_documentation(&mxmldoc);
        }
      }
      else
      {
       /*
        * Load source file...
	*/

        update = 1;

	if (!doc)
	  doc = new_documentation(&mxmldoc);

	if ((fp = fopen(argv[i], "r")) == NULL)
	{
	  fprintf(stderr, "mxmldoc: Unable to open source file \"%s\": %s\n",
	          argv[i], strerror(errno));
	  mxmlDelete(doc);
	  return (1);
	}
	else if (scan_file(argv[i], fp, mxmldoc))
	{
	  fclose(fp);
	  mxmlDelete(doc);
	  return (1);
	}
	else
	  fclose(fp);
      }
    }

  if (update && xmlfile)
  {
   /*
    * Save the updated XML documentation file...
    */

    if ((fp = fopen(xmlfile, "w")) != NULL)
    {
     /*
      * Write over the existing XML file...
      */

      if (mxmlSaveFile(doc, fp, ws_cb))
      {
	fprintf(stderr, "mxmldoc: Unable to write the XML documentation file \"%s\": %s!\n",
        	xmlfile, strerror(errno));
	fclose(fp);
	mxmlDelete(doc);
	return (1);
      }

      fclose(fp);
    }
    else
    {
      fprintf(stderr, "mxmldoc: Unable to create the XML documentation file \"%s\": %s!\n",
              xmlfile, strerror(errno));
      mxmlDelete(doc);
      return (1);
    }
  }

  switch (mode)
  {
    case OUTPUT_HTML :
       /*
        * Write HTML documentation...
        */

        write_html(section, title ? title : "Documentation", introfile,
                   mxmldoc);
        break;

    case OUTPUT_MAN :
       /*
        * Write manpage documentation...
        */

        write_man(name, section, title, introfile, mxmldoc);
        break;
  }
  
 /*
  * Delete the tree and return...
  */

  mxmlDelete(doc);

  return (0);
}


/*
 * 'add_variable()' - Add a variable or argument.
 */

static mxml_node_t *			/* O - New variable/argument */
add_variable(mxml_node_t *parent,	/* I - Parent node */
             const char  *name,		/* I - "argument" or "variable" */
             mxml_node_t *type)		/* I - Type nodes */
{
  mxml_node_t	*variable,		/* New variable */
		*node,			/* Current node */
		*next;			/* Next node */
  char		buffer[16384],		/* String buffer */
		*bufptr;		/* Pointer into buffer */


#ifdef DEBUG
  fprintf(stderr, "add_variable(parent=%p, name=\"%s\", type=%p)\n",
          parent, name, type);
#endif /* DEBUG */

 /*
  * Range check input...
  */

  if (!type || !type->child)
    return (NULL);

 /*
  * Create the variable/argument node...
  */

  variable = mxmlNewElement(parent, name);

 /*
  * Check for a default value...
  */

  for (node = type->child; node; node = node->next)
    if (!strcmp(node->value.text.string, "="))
      break;

  if (node)
  {
   /*
    * Default value found, copy it and add as a "default" attribute...
    */

    for (bufptr = buffer; node; bufptr += strlen(bufptr))
    {
      if (node->value.text.whitespace && bufptr > buffer)
	*bufptr++ = ' ';

      strcpy(bufptr, node->value.text.string);

      next = node->next;
      mxmlDelete(node);
      node = next;
    }

    mxmlElementSetAttr(variable, "default", buffer);
  }

 /*
  * Extract the argument/variable name...
  */

  if (type->last_child->value.text.string[0] == ')')
  {
   /*
    * Handle "type (*name)(args)"...
    */

    for (node = type->child; node; node = node->next)
      if (node->value.text.string[0] == '(')
	break;

    for (bufptr = buffer; node; bufptr += strlen(bufptr))
    {
      if (node->value.text.whitespace && bufptr > buffer)
	*bufptr++ = ' ';

      strcpy(bufptr, node->value.text.string);

      next = node->next;
      mxmlDelete(node);
      node = next;
    }
  }
  else
  {
   /*
    * Handle "type name"...
    */

    strcpy(buffer, type->last_child->value.text.string);
    mxmlDelete(type->last_child);
  }

 /*
  * Set the name...
  */

  mxmlElementSetAttr(variable, "name", buffer);

 /*
  * Add the remaining type information to the variable node...
  */

  mxmlAdd(variable, MXML_ADD_AFTER, MXML_ADD_TO_PARENT, type);

 /*
  * Add new new variable node...
  */

  return (variable);
}


/*
 * 'find_public()' - Find a public function, type, etc.
 */

static mxml_node_t *			/* I - Found node or NULL */
find_public(mxml_node_t *node,		/* I - Current node */
            mxml_node_t *top,		/* I - Top node */
            const char  *name)		/* I - Name of element */
{
  mxml_node_t	*description,		/* Description node */
		*comment;		/* Comment node */


  for (node = mxmlFindElement(node, top, name, NULL, NULL,
                              node == top ? MXML_DESCEND_FIRST :
			                    MXML_NO_DESCEND);
       node;
       node = mxmlFindElement(node, top, name, NULL, NULL, MXML_NO_DESCEND))
  {
   /*
    * Get the description for this node...
    */

    description = mxmlFindElement(node, node, "description", NULL, NULL,
                                  MXML_DESCEND_FIRST);

   /*
    * A missing or empty description signals a private node...
    */

#if 0
    if (!description || !description->child)
      continue;
#else
    if (!description)
      continue;
#endif /* 0 */

   /*
    * Look for @private@ in the comment text...
    */

    for (comment = description->child; comment; comment = comment->next)
      if ((comment->type == MXML_TEXT &&
           strstr(comment->value.text.string, "@private@")) ||
          (comment->type == MXML_OPAQUE &&
           strstr(comment->value.opaque, "@private@")))
        break;

    if (!comment)
    {
     /*
      * No @private@, so return this node...
      */

      return (node);
    }
  }

 /*
  * If we get here, there are no (more) public nodes...
  */

  return (NULL);
}


/*
 * 'get_comment_info()' - Get info from comment.
 */

static char *				/* O - Info from comment */
get_comment_info(
    mxml_node_t *description)		/* I - Description node */
{
  char		text[10240],		/* Description text */
		since[255],		/* @since value */
		*ptr;			/* Pointer into text */
  static char	info[1024];		/* Info string */


  if (!description)
    return ("");

  get_text(description, text, sizeof(text));

  for (ptr = strchr(text, '@'); ptr; ptr = strchr(ptr + 1, '@'))
  {
    if (!strncmp(ptr, "@deprecated@", 12))
      return ("<span class='info'>&nbsp;DEPRECATED&nbsp;</span>");
    else if (!strncmp(ptr, "@since ", 7))
    {
      strncpy(since, ptr + 7, sizeof(since) - 1);
      since[sizeof(since) - 1] = '\0';

      if ((ptr = strchr(since, '@')) != NULL)
        *ptr = '\0';

      snprintf(info, sizeof(info), "<span class='info'>&nbsp;%s&nbsp;</span>", since);
      return (info);
    }
  }

  return ("");
}


/*
 * 'get_text()' - Get the text for a node.
 */

static char *				/* O - Text in node */
get_text(mxml_node_t *node,		/* I - Node to get */
         char        *buffer,		/* I - Buffer */
	 int         buflen)		/* I - Size of buffer */
{
  char		*ptr,			/* Pointer into buffer */
		*end;			/* End of buffer */
  int		len;			/* Length of node */
  mxml_node_t	*current;		/* Current node */


  ptr = buffer;
  end = buffer + buflen - 1;

  for (current = node->child; current && ptr < end; current = current->next)
  {
    if (current->type == MXML_TEXT)
    {
      if (current->value.text.whitespace)
        *ptr++ = ' ';

      len = (int)strlen(current->value.text.string);
      if (len > (int)(end - ptr))
        len = (int)(end - ptr);

      memcpy(ptr, current->value.text.string, len);
      ptr += len;
    }
    else if (current->type == MXML_OPAQUE)
    {
      len = (int)strlen(current->value.opaque);
      if (len > (int)(end - ptr))
        len = (int)(end - ptr);

      memcpy(ptr, current->value.opaque, len);
      ptr += len;
    }
  }

  *ptr = '\0';

  return (buffer);
}


/*
 * 'load_cb()' - Set the type of child nodes.
 */

static mxml_type_t			/* O - Node type */
load_cb(mxml_node_t *node)		/* I - Node */
{
  if (!strcmp(node->value.element.name, "description"))
    return (MXML_OPAQUE);
  else
    return (MXML_TEXT);
}


/*
 * 'new_documentation()' - Create a new documentation tree.
 */

static mxml_node_t *			/* O - New documentation */
new_documentation(mxml_node_t **mxmldoc)/* O - mxmldoc node */
{
  mxml_node_t	*doc;			/* New documentation */


 /*
  * Create an empty XML documentation file...
  */

  doc = mxmlNewXML(NULL);

  *mxmldoc = mxmlNewElement(doc, "mxmldoc");

  mxmlElementSetAttr(*mxmldoc, "xmlns", "http://www.easysw.com");
  mxmlElementSetAttr(*mxmldoc, "xmlns:xsi",
                     "http://www.w3.org/2001/XMLSchema-instance");
  mxmlElementSetAttr(*mxmldoc, "xsi:schemaLocation",
                     "http://www.minixml.org/mxmldoc.xsd");

  return (doc);
}

/*
 * 'safe_strcpy()' - Copy a string allowing for overlapping strings.
 */

static void
safe_strcpy(char       *dst,		/* I - Destination string */
            const char *src)		/* I - Source string */
{
  while (*src)
    *dst++ = *src++;

  *dst = '\0';
}


/*
 * 'scan_file()' - Scan a source file.
 */

static int				/* O - 0 on success, -1 on error */
scan_file(const char  *filename,	/* I - Filename */
          FILE        *fp,		/* I - File to scan */
          mxml_node_t *tree)		/* I - Function tree */
{
  int		state,			/* Current parser state */
		braces,			/* Number of braces active */
		parens;			/* Number of active parenthesis */
  int		ch;			/* Current character */
  char		buffer[65536],		/* String buffer */
		*bufptr;		/* Pointer into buffer */
  const char	*scope;			/* Current variable/function scope */
  mxml_node_t	*comment,		/* <comment> node */
		*constant,		/* <constant> node */
		*enumeration,		/* <enumeration> node */
		*function,		/* <function> node */
		*fstructclass,		/* function struct/class node */
		*structclass,		/* <struct> or <class> node */
		*typedefnode,		/* <typedef> node */
		*variable,		/* <variable> or <argument> node */
		*returnvalue,		/* <returnvalue> node */
		*type,			/* <type> node */
		*description,		/* <description> node */
		*node,			/* Current node */
		*next;			/* Next node */
#if DEBUG > 1
  mxml_node_t	*temp;			/* Temporary node */
  int		oldstate,		/* Previous state */
		oldch;			/* Old character */
  static const char *states[] =		/* State strings */
		{
		  "STATE_NONE",
		  "STATE_PREPROCESSOR",
		  "STATE_C_COMMENT",
		  "STATE_CXX_COMMENT",
		  "STATE_STRING",
		  "STATE_CHARACTER",
		  "STATE_IDENTIFIER"
		};
#endif /* DEBUG > 1 */


#ifdef DEBUG
  fprintf(stderr, "scan_file(filename=\"%s\", fp=%p, tree=%p)\n", filename,
          fp, tree);
#endif /* DEBUG */

 /*
  * Initialize the finite state machine...
  */

  state        = STATE_NONE;
  braces       = 0;
  parens       = 0;
  bufptr       = buffer;

  comment      = mxmlNewElement(MXML_NO_PARENT, "temp");
  constant     = NULL;
  enumeration  = NULL;
  function     = NULL;
  variable     = NULL;
  returnvalue  = NULL;
  type         = NULL;
  description  = NULL;
  typedefnode  = NULL;
  structclass  = NULL;
  fstructclass = NULL;

  if (!strcmp(tree->value.element.name, "class"))
    scope = "private";
  else
    scope = NULL;

 /*
  * Read until end-of-file...
  */

  while ((ch = getc(fp)) != EOF)
  {
#if DEBUG > 1
    oldstate = state;
    oldch    = ch;
#endif /* DEBUG > 1 */

    switch (state)
    {
      case STATE_NONE :			/* No state - whitespace, etc. */
          switch (ch)
	  {
	    case '/' :			/* Possible C/C++ comment */
	        ch     = getc(fp);
		bufptr = buffer;

		if (ch == '*')
		  state = STATE_C_COMMENT;
		else if (ch == '/')
		  state = STATE_CXX_COMMENT;
		else
		{
		  ungetc(ch, fp);

		  if (type)
		  {
#ifdef DEBUG
                    fputs("Identifier: <<<< / >>>\n", stderr);
#endif /* DEBUG */
                    ch = type->last_child->value.text.string[0];
		    mxmlNewText(type, isalnum(ch) || ch == '_', "/");
		  }
		}
		break;

	    case '#' :			/* Preprocessor */
#ifdef DEBUG
	        fputs("    #preprocessor...\n", stderr);
#endif /* DEBUG */
	        state = STATE_PREPROCESSOR;
		break;

            case '\'' :			/* Character constant */
	        state = STATE_CHARACTER;
		bufptr = buffer;
		*bufptr++ = ch;
		break;

            case '\"' :			/* String constant */
	        state = STATE_STRING;
		bufptr = buffer;
		*bufptr++ = ch;
		break;

            case '{' :
#ifdef DEBUG
	        fprintf(stderr, "    open brace, function=%p, type=%p...\n",
		        function, type);
                if (type)
                  fprintf(stderr, "    type->child=\"%s\"...\n",
		          type->child->value.text.string);
#endif /* DEBUG */

	        if (function)
		{
		  if (fstructclass)
		  {
		    sort_node(fstructclass, function);
		    fstructclass = NULL;
		  }
		  else
		    sort_node(tree, function);

		  function = NULL;
		}
		else if (type && type->child &&
		         ((!strcmp(type->child->value.text.string, "typedef") &&
			   type->child->next &&
			   (!strcmp(type->child->next->value.text.string, "struct") ||
			    !strcmp(type->child->next->value.text.string, "union") ||
			    !strcmp(type->child->next->value.text.string, "class"))) ||
			  !strcmp(type->child->value.text.string, "union") ||
			  !strcmp(type->child->value.text.string, "struct") ||
			  !strcmp(type->child->value.text.string, "class")))
		{
		 /*
		  * Start of a class or structure...
		  */

		  if (!strcmp(type->child->value.text.string, "typedef"))
		  {
#ifdef DEBUG
                    fputs("    starting typedef...\n", stderr);
#endif /* DEBUG */

		    typedefnode = mxmlNewElement(MXML_NO_PARENT, "typedef");
		    mxmlDelete(type->child);
		  }
		  else
		    typedefnode = NULL;
	
		  structclass = mxmlNewElement(MXML_NO_PARENT,
		                               type->child->value.text.string);

#ifdef DEBUG
                  fprintf(stderr, "%c%s: <<<< %s >>>\n",
		          toupper(type->child->value.text.string[0]),
			  type->child->value.text.string + 1,
			  type->child->next ?
			      type->child->next->value.text.string : "(noname)");

                  fputs("    type =", stderr);
                  for (node = type->child; node; node = node->next)
		    fprintf(stderr, " \"%s\"", node->value.text.string);
		  putc('\n', stderr);

                  fprintf(stderr, "    scope = %s\n", scope ? scope : "(null)");
#endif /* DEBUG */

                  if (type->child->next)
		  {
		    mxmlElementSetAttr(structclass, "name",
		                       type->child->next->value.text.string);
		    sort_node(tree, structclass);
		  }

                  if (typedefnode && type->child)
		    type->child->value.text.whitespace = 0;
                  else if (structclass && type->child &&
		           type->child->next && type->child->next->next)
		  {
		    for (bufptr = buffer, node = type->child->next->next;
		         node;
			 bufptr += strlen(bufptr))
		    {
		      if (node->value.text.whitespace && bufptr > buffer)
			*bufptr++ = ' ';

		      strcpy(bufptr, node->value.text.string);

		      next = node->next;
		      mxmlDelete(node);
		      node = next;
		    }

		    mxmlElementSetAttr(structclass, "parent", buffer);

		    mxmlDelete(type);
		    type = NULL;
		  }
		  else
		  {
		    mxmlDelete(type);
		    type = NULL;
		  }

		  if (typedefnode && comment->last_child)
		  {
		   /*
		    * Copy comment for typedef as well as class/struct/union...
		    */

		    mxmlNewText(comment, 0,
		                comment->last_child->value.text.string);
		    description = mxmlNewElement(typedefnode, "description");
#ifdef DEBUG
		    fputs("    duplicating comment for typedef...\n", stderr);
#endif /* DEBUG */
		    update_comment(typedefnode, comment->last_child);
		    mxmlAdd(description, MXML_ADD_AFTER, MXML_ADD_TO_PARENT,
		            comment->last_child);
		  }

		  description = mxmlNewElement(structclass, "description");
#ifdef DEBUG
		  fprintf(stderr, "    adding comment to %s...\n",
		          structclass->value.element.name);
#endif /* DEBUG */
		  update_comment(structclass, comment->last_child);
		  mxmlAdd(description, MXML_ADD_AFTER, MXML_ADD_TO_PARENT,
		          comment->last_child);

                  if (scan_file(filename, fp, structclass))
		  {
		    mxmlDelete(comment);
		    return (-1);
		  }

#ifdef DEBUG
                  fputs("    ended typedef...\n", stderr);
#endif /* DEBUG */
                  structclass = NULL;
                  break;
                }
		else if (type && type->child && type->child->next &&
		         (!strcmp(type->child->value.text.string, "enum") ||
			  (!strcmp(type->child->value.text.string, "typedef") &&
			   !strcmp(type->child->next->value.text.string, "enum"))))
                {
		 /*
		  * Enumeration type...
		  */

		  if (!strcmp(type->child->value.text.string, "typedef"))
		  {
#ifdef DEBUG
                    fputs("    starting typedef...\n", stderr);
#endif /* DEBUG */

		    typedefnode = mxmlNewElement(MXML_NO_PARENT, "typedef");
		    mxmlDelete(type->child);
		  }
		  else
		    typedefnode = NULL;
	
		  enumeration = mxmlNewElement(MXML_NO_PARENT, "enumeration");

#ifdef DEBUG
                  fprintf(stderr, "Enumeration: <<<< %s >>>\n",
			  type->child->next ?
			      type->child->next->value.text.string : "(noname)");
#endif /* DEBUG */

                  if (type->child->next)
		  {
		    mxmlElementSetAttr(enumeration, "name",
		                       type->child->next->value.text.string);
		    sort_node(tree, enumeration);
		  }

                  if (typedefnode && type->child)
		    type->child->value.text.whitespace = 0;
                  else
		  {
		    mxmlDelete(type);
		    type = NULL;
		  }

		  if (typedefnode && comment->last_child)
		  {
		   /*
		    * Copy comment for typedef as well as class/struct/union...
		    */

		    mxmlNewText(comment, 0,
		                comment->last_child->value.text.string);
		    description = mxmlNewElement(typedefnode, "description");
#ifdef DEBUG
		    fputs("    duplicating comment for typedef...\n", stderr);
#endif /* DEBUG */
		    update_comment(typedefnode, comment->last_child);
		    mxmlAdd(description, MXML_ADD_AFTER, MXML_ADD_TO_PARENT,
		            comment->last_child);
		  }

		  description = mxmlNewElement(enumeration, "description");
#ifdef DEBUG
		  fputs("    adding comment to enumeration...\n", stderr);
#endif /* DEBUG */
		  update_comment(enumeration, comment->last_child);
		  mxmlAdd(description, MXML_ADD_AFTER, MXML_ADD_TO_PARENT,
		          comment->last_child);
		}
		else if (type && type->child &&
		         !strcmp(type->child->value.text.string, "extern"))
                {
                  if (scan_file(filename, fp, tree))
		  {
		    mxmlDelete(comment);
		    return (-1);
		  }
                }
		else if (type)
		{
		  mxmlDelete(type);
		  type = NULL;
		}

	        braces ++;
		function = NULL;
		variable = NULL;
		break;

            case '}' :
#ifdef DEBUG
	        fputs("    close brace...\n", stderr);
#endif /* DEBUG */

                if (structclass)
		  scope = NULL;

                enumeration = NULL;
		constant    = NULL;
		structclass = NULL;

	        if (braces > 0)
		  braces --;
		else
		{
		  mxmlDelete(comment);
		  return (0);
		}
		break;

            case '(' :
		if (type)
		{
#ifdef DEBUG
                  fputs("Identifier: <<<< ( >>>\n", stderr);
#endif /* DEBUG */
		  mxmlNewText(type, 0, "(");
		}

	        parens ++;
		break;

            case ')' :
		if (type && parens)
		{
#ifdef DEBUG
                  fputs("Identifier: <<<< ) >>>\n", stderr);
#endif /* DEBUG */
		  mxmlNewText(type, 0, ")");
		}

                if (function && type && !parens)
		{
		 /*
		  * Check for "void" argument...
		  */

		  if (type->child && type->child->next)
		    variable = add_variable(function, "argument", type);
		  else
		    mxmlDelete(type);

		  type = NULL;
		}

	        if (parens > 0)
		  parens --;
		break;

	    case ';' :
#ifdef DEBUG
                fputs("Identifier: <<<< ; >>>\n", stderr);
		fprintf(stderr, "    function=%p, type=%p\n", function, type);
#endif /* DEBUG */

	        if (function)
		{
		  if (!strcmp(tree->value.element.name, "class"))
		  {
#ifdef DEBUG
		    fputs("    ADDING FUNCTION TO CLASS\n", stderr);
#endif /* DEBUG */
		    sort_node(tree, function);
		  }
		  else
		    mxmlDelete(function);

		  function = NULL;
		  variable = NULL;
		}

		if (type)
		{
		 /*
		  * See if we have a function typedef...
		  */

		  if (type->child &&
		      !strcmp(type->child->value.text.string, "typedef"))
		  {
		   /*
		    * Yes, add it!
		    */

		    typedefnode = mxmlNewElement(MXML_NO_PARENT, "typedef");

		    for (node = type->child->next; node; node = node->next)
		      if (!strcmp(node->value.text.string, "("))
			break;

		    for (node = node->next; node; node = node->next)
		      if (strcmp(node->value.text.string, "*"))
			break;

                    if (node)
		    {
		      mxmlElementSetAttr(typedefnode, "name",
		                         node->value.text.string);
                      sort_node(tree, typedefnode);

		      mxmlDelete(type->child);
		      mxmlDelete(node);

                      if (type->child)
			type->child->value.text.whitespace = 0;

		      mxmlAdd(typedefnode, MXML_ADD_AFTER, MXML_ADD_TO_PARENT,
		              type);
		      type = NULL;
		      break;
		    }
		  }
		  
		  mxmlDelete(type);
		  type = NULL;
		}
		break;

	    case ':' :
		if (type)
		{
#ifdef DEBUG
                  fputs("Identifier: <<<< : >>>\n", stderr);
#endif /* DEBUG */
		  mxmlNewText(type, 1, ":");
		}
		break;

	    case '*' :
		if (type)
		{
#ifdef DEBUG
                  fputs("Identifier: <<<< * >>>\n", stderr);
#endif /* DEBUG */
                  ch = type->last_child->value.text.string[0];
		  mxmlNewText(type, isalnum(ch) || ch == '_', "*");
		}
		break;

	    case ',' :
		if (type && !enumeration)
		{
#ifdef DEBUG
                  fputs("Identifier: <<<< , >>>\n", stderr);
#endif /* DEBUG */
                  ch = type->last_child->value.text.string[0];
		  mxmlNewText(type, 0, ",");
		}
		break;

	    case '&' :
		if (type)
		{
#ifdef DEBUG
                  fputs("Identifier: <<<< & >>>\n", stderr);
#endif /* DEBUG */
		  mxmlNewText(type, 1, "&");
		}
		break;

	    case '+' :
		if (type)
		{
#ifdef DEBUG
                  fputs("Identifier: <<<< + >>>\n", stderr);
#endif /* DEBUG */
                  ch = type->last_child->value.text.string[0];
		  mxmlNewText(type, isalnum(ch) || ch == '_', "+");
		}
		break;

	    case '-' :
		if (type)
		{
#ifdef DEBUG
                  fputs("Identifier: <<<< - >>>\n", stderr);
#endif /* DEBUG */
                  ch = type->last_child->value.text.string[0];
		  mxmlNewText(type, isalnum(ch) || ch == '_', "-");
		}
		break;

	    case '=' :
		if (type)
		{
#ifdef DEBUG
                  fputs("Identifier: <<<< = >>>\n", stderr);
#endif /* DEBUG */
                  ch = type->last_child->value.text.string[0];
		  mxmlNewText(type, isalnum(ch) || ch == '_', "=");
		}
		break;

            default :			/* Other */
	        if (isalnum(ch) || ch == '_' || ch == '.' || ch == ':' || ch == '~')
		{
		  state     = STATE_IDENTIFIER;
		  bufptr    = buffer;
		  *bufptr++ = ch;
		}
		break;
          }
          break;

      case STATE_PREPROCESSOR :		/* Preprocessor directive */
          if (ch == '\n')
	    state = STATE_NONE;
	  else if (ch == '\\')
	    getc(fp);
          break;

      case STATE_C_COMMENT :		/* Inside a C comment */
          switch (ch)
	  {
	    case '\n' :
	        while ((ch = getc(fp)) != EOF)
		  if (ch == '*')
		  {
		    ch = getc(fp);

		    if (ch == '/')
		    {
		      *bufptr = '\0';

        	      if (comment->child != comment->last_child)
		      {
#ifdef DEBUG
			fprintf(stderr, "    removing comment %p, last comment %p...\n",
				comment->child, comment->last_child);
#endif /* DEBUG */
			mxmlDelete(comment->child);
#ifdef DEBUG
			fprintf(stderr, "    new comment %p, last comment %p...\n",
				comment->child, comment->last_child);
#endif /* DEBUG */
		      }

#ifdef DEBUG
                      fprintf(stderr,
		              "    processing comment, variable=%p, "
		              "constant=%p, typedefnode=%p, tree=\"%s\"\n",
		              variable, constant, typedefnode,
			      tree->value.element.name);
#endif /* DEBUG */

		      if (variable)
		      {
        		description = mxmlNewElement(variable, "description");
#ifdef DEBUG
			fputs("    adding comment to variable...\n", stderr);
#endif /* DEBUG */
			update_comment(variable,
			               mxmlNewText(description, 0, buffer));
                        variable = NULL;
		      }
		      else if (constant)
		      {
        		description = mxmlNewElement(constant, "description");
#ifdef DEBUG
		        fputs("    adding comment to constant...\n", stderr);
#endif /* DEBUG */
			update_comment(constant,
			               mxmlNewText(description, 0, buffer));
                        constant = NULL;
		      }
		      else if (typedefnode)
		      {
        		description = mxmlNewElement(typedefnode, "description");
#ifdef DEBUG
			fprintf(stderr, "    adding comment to typedef %s...\n",
			        mxmlElementGetAttr(typedefnode, "name"));
#endif /* DEBUG */
			update_comment(typedefnode,
			               mxmlNewText(description, 0, buffer));

                	if (structclass)
			{
        		  description = mxmlNewElement(structclass, "description");
			  update_comment(structclass,
			        	 mxmlNewText(description, 0, buffer));
                	}
			else if (enumeration)
			{
        		  description = mxmlNewElement(enumeration, "description");
			  update_comment(enumeration,
			        	 mxmlNewText(description, 0, buffer));
			}

			typedefnode = NULL;
		      }
		      else if (strcmp(tree->value.element.name, "mxmldoc") &&
		               !mxmlFindElement(tree, tree, "description",
			                        NULL, NULL, MXML_DESCEND_FIRST))
                      {
        		description = mxmlNewElement(tree, "description");
#ifdef DEBUG
			fputs("    adding comment to parent...\n", stderr);
#endif /* DEBUG */
			update_comment(tree,
			               mxmlNewText(description, 0, buffer));
		      }
		      else
		      {
#ifdef DEBUG
		        fprintf(stderr, "    before adding comment, child=%p, last_child=%p\n",
			        comment->child, comment->last_child);
#endif /* DEBUG */
        		mxmlNewText(comment, 0, buffer);
#ifdef DEBUG
		        fprintf(stderr, "    after adding comment, child=%p, last_child=%p\n",
			        comment->child, comment->last_child);
#endif /* DEBUG */
                      }
#ifdef DEBUG
		      fprintf(stderr, "C comment: <<<< %s >>>\n", buffer);
#endif /* DEBUG */

		      state = STATE_NONE;
		      break;
		    }
		    else
		      ungetc(ch, fp);
		  }
		  else if (ch == '\n' && bufptr > buffer &&
		           bufptr < (buffer + sizeof(buffer) - 1))
		    *bufptr++ = ch;
		  else if (!isspace(ch))
		    break;

		if (ch != EOF)
		  ungetc(ch, fp);

                if (bufptr > buffer && bufptr < (buffer + sizeof(buffer) - 1))
		  *bufptr++ = '\n';
		break;

	    case '/' :
	        if (ch == '/' && bufptr > buffer && bufptr[-1] == '*')
		{
		  while (bufptr > buffer &&
		         (bufptr[-1] == '*' || isspace(bufptr[-1] & 255)))
		    bufptr --;
		  *bufptr = '\0';

        	  if (comment->child != comment->last_child)
		  {
#ifdef DEBUG
		    fprintf(stderr, "    removing comment %p, last comment %p...\n",
			    comment->child, comment->last_child);
#endif /* DEBUG */
		    mxmlDelete(comment->child);
#ifdef DEBUG
		    fprintf(stderr, "    new comment %p, last comment %p...\n",
			    comment->child, comment->last_child);
#endif /* DEBUG */
		  }

#ifdef DEBUG
                  fprintf(stderr,
		          "    processing comment, variable=%p, "
		          "constant=%p, typedefnode=%p, tree=\"%s\"\n",
		          variable, constant, typedefnode,
			  tree->value.element.name);
#endif /* DEBUG */

		  if (variable)
		  {
        	    description = mxmlNewElement(variable, "description");
#ifdef DEBUG
		    fputs("    adding comment to variable...\n", stderr);
#endif /* DEBUG */
		    update_comment(variable,
			           mxmlNewText(description, 0, buffer));
                    variable = NULL;
		  }
		  else if (constant)
		  {
        	    description = mxmlNewElement(constant, "description");
#ifdef DEBUG
		    fputs("    adding comment to constant...\n", stderr);
#endif /* DEBUG */
		    update_comment(constant,
			           mxmlNewText(description, 0, buffer));
		    constant = NULL;
		  }
		  else if (typedefnode)
		  {
        	    description = mxmlNewElement(typedefnode, "description");
#ifdef DEBUG
		    fprintf(stderr, "    adding comment to typedef %s...\n",
			    mxmlElementGetAttr(typedefnode, "name"));
#endif /* DEBUG */
		    update_comment(typedefnode,
			           mxmlNewText(description, 0, buffer));

                    if (structclass)
		    {
        	      description = mxmlNewElement(structclass, "description");
		      update_comment(structclass,
			             mxmlNewText(description, 0, buffer));
                    }
		    else if (enumeration)
		    {
        	      description = mxmlNewElement(enumeration, "description");
		      update_comment(enumeration,
			             mxmlNewText(description, 0, buffer));
		    }

		    typedefnode = NULL;
		  }
		  else if (strcmp(tree->value.element.name, "mxmldoc") &&
		           !mxmlFindElement(tree, tree, "description",
			                    NULL, NULL, MXML_DESCEND_FIRST))
                  {
        	    description = mxmlNewElement(tree, "description");
#ifdef DEBUG
		    fputs("    adding comment to parent...\n", stderr);
#endif /* DEBUG */
		    update_comment(tree,
			           mxmlNewText(description, 0, buffer));
		  }
		  else
        	    mxmlNewText(comment, 0, buffer);

#ifdef DEBUG
		  fprintf(stderr, "C comment: <<<< %s >>>\n", buffer);
#endif /* DEBUG */

		  state = STATE_NONE;
		  break;
		}

	    default :
	        if (ch == ' ' && bufptr == buffer)
		  break;

	        if (bufptr < (buffer + sizeof(buffer) - 1))
		  *bufptr++ = ch;
		break;
          }
          break;

      case STATE_CXX_COMMENT :		/* Inside a C++ comment */
          if (ch == '\n')
	  {
	    state = STATE_NONE;
	    *bufptr = '\0';

            if (comment->child != comment->last_child)
	    {
#ifdef DEBUG
	      fprintf(stderr, "    removing comment %p, last comment %p...\n",
		      comment->child, comment->last_child);
#endif /* DEBUG */
	      mxmlDelete(comment->child);
#ifdef DEBUG
	      fprintf(stderr, "    new comment %p, last comment %p...\n",
		      comment->child, comment->last_child);
#endif /* DEBUG */
	    }

	    if (variable)
	    {
              description = mxmlNewElement(variable, "description");
#ifdef DEBUG
	      fputs("    adding comment to variable...\n", stderr);
#endif /* DEBUG */
	      update_comment(variable,
			     mxmlNewText(description, 0, buffer));
              variable = NULL;
	    }
	    else if (constant)
	    {
              description = mxmlNewElement(constant, "description");
#ifdef DEBUG
	      fputs("    adding comment to constant...\n", stderr);
#endif /* DEBUG */
	      update_comment(constant,
			     mxmlNewText(description, 0, buffer));
              constant = NULL;
	    }
	    else if (typedefnode)
	    {
              description = mxmlNewElement(typedefnode, "description");
#ifdef DEBUG
	      fprintf(stderr, "    adding comment to typedef %s...\n",
		      mxmlElementGetAttr(typedefnode, "name"));
#endif /* DEBUG */
	      update_comment(typedefnode,
			     mxmlNewText(description, 0, buffer));

              if (structclass)
	      {
        	description = mxmlNewElement(structclass, "description");
		update_comment(structclass,
			       mxmlNewText(description, 0, buffer));
              }
	      else if (enumeration)
	      {
        	description = mxmlNewElement(enumeration, "description");
		update_comment(enumeration,
			       mxmlNewText(description, 0, buffer));
	      }
	    }
	    else if (strcmp(tree->value.element.name, "mxmldoc") &&
		     !mxmlFindElement(tree, tree, "description",
			              NULL, NULL, MXML_DESCEND_FIRST))
            {
              description = mxmlNewElement(tree, "description");
#ifdef DEBUG
	      fputs("    adding comment to parent...\n", stderr);
#endif /* DEBUG */
	      update_comment(tree,
			     mxmlNewText(description, 0, buffer));
	    }
	    else
              mxmlNewText(comment, 0, buffer);

#ifdef DEBUG
	    fprintf(stderr, "C++ comment: <<<< %s >>>\n", buffer);
#endif /* DEBUG */
	  }
	  else if (ch == ' ' && bufptr == buffer)
	    break;
	  else if (bufptr < (buffer + sizeof(buffer) - 1))
	    *bufptr++ = ch;
          break;

      case STATE_STRING :		/* Inside a string constant */
	  *bufptr++ = ch;

          if (ch == '\\')
	    *bufptr++ = getc(fp);
	  else if (ch == '\"')
	  {
	    *bufptr = '\0';

	    if (type)
	      mxmlNewText(type, type->child != NULL, buffer);

	    state = STATE_NONE;
	  }
          break;

      case STATE_CHARACTER :		/* Inside a character constant */
	  *bufptr++ = ch;

          if (ch == '\\')
	    *bufptr++ = getc(fp);
	  else if (ch == '\'')
	  {
	    *bufptr = '\0';

	    if (type)
	      mxmlNewText(type, type->child != NULL, buffer);

	    state = STATE_NONE;
	  }
          break;

      case STATE_IDENTIFIER :		/* Inside a keyword or identifier */
	  if (isalnum(ch) || ch == '_' || ch == '[' || ch == ']' ||
	      (ch == ',' && (parens > 1 || (type && !enumeration && !function))) ||
	      ch == ':' || ch == '.' || ch == '~')
	  {
	    if (bufptr < (buffer + sizeof(buffer) - 1))
	      *bufptr++ = ch;
	  }
	  else
	  {
	    ungetc(ch, fp);
	    *bufptr = '\0';
	    state   = STATE_NONE;

#ifdef DEBUG
            fprintf(stderr, "    braces=%d, type=%p, type->child=%p, buffer=\"%s\"\n",
	            braces, type, type ? type->child : NULL, buffer);
#endif /* DEBUG */

            if (!braces)
	    {
	      if (!type || !type->child)
	      {
		if (!strcmp(tree->value.element.name, "class"))
		{
		  if (!strcmp(buffer, "public") ||
	              !strcmp(buffer, "public:"))
		  {
		    scope = "public";
#ifdef DEBUG
		    fputs("    scope = public\n", stderr);
#endif /* DEBUG */
		    break;
		  }
		  else if (!strcmp(buffer, "private") ||
	                   !strcmp(buffer, "private:"))
		  {
		    scope = "private";
#ifdef DEBUG
		    fputs("    scope = private\n", stderr);
#endif /* DEBUG */
		    break;
		  }
		  else if (!strcmp(buffer, "protected") ||
	                   !strcmp(buffer, "protected:"))
		  {
		    scope = "protected";
#ifdef DEBUG
		    fputs("    scope = protected\n", stderr);
#endif /* DEBUG */
		    break;
		  }
		}
	      }

	      if (!type)
                type = mxmlNewElement(MXML_NO_PARENT, "type");

#ifdef DEBUG
              fprintf(stderr, "    function=%p (%s), type->child=%p, ch='%c', parens=%d\n",
	              function,
		      function ? mxmlElementGetAttr(function, "name") : "null",
	              type->child, ch, parens);
#endif /* DEBUG */

              if (!function && ch == '(')
	      {
	        if (type->child &&
		    !strcmp(type->child->value.text.string, "extern"))
		{
		 /*
		  * Remove external declarations...
		  */

		  mxmlDelete(type);
		  type = NULL;
		  break;
		}

	        if (type->child &&
		    !strcmp(type->child->value.text.string, "static") &&
		    !strcmp(tree->value.element.name, "mxmldoc"))
		{
		 /*
		  * Remove static functions...
		  */

		  mxmlDelete(type);
		  type = NULL;
		  break;
		}

	        function = mxmlNewElement(MXML_NO_PARENT, "function");
		if ((bufptr = strchr(buffer, ':')) != NULL && bufptr[1] == ':')
		{
		  *bufptr = '\0';
		  bufptr += 2;

		  if ((fstructclass =
		           mxmlFindElement(tree, tree, "class", "name", buffer,
		                           MXML_DESCEND_FIRST)) == NULL)
		    fstructclass =
		        mxmlFindElement(tree, tree, "struct", "name", buffer,
		                        MXML_DESCEND_FIRST);
		}
		else
		  bufptr = buffer;

		mxmlElementSetAttr(function, "name", bufptr);

		if (scope)
		  mxmlElementSetAttr(function, "scope", scope);

#ifdef DEBUG
                fprintf(stderr, "function: %s\n", buffer);
		fprintf(stderr, "    scope = %s\n", scope ? scope : "(null)");
		fprintf(stderr, "    comment = %p\n", comment);
		fprintf(stderr, "    child = (%p) %s\n",
		        comment->child,
			comment->child ?
			    comment->child->value.text.string : "(null)");
		fprintf(stderr, "    last_child = (%p) %s\n",
		        comment->last_child,
			comment->last_child ?
			    comment->last_child->value.text.string : "(null)");
#endif /* DEBUG */

                if (type->last_child &&
		    strcmp(type->last_child->value.text.string, "void"))
		{
                  returnvalue = mxmlNewElement(function, "returnvalue");

		  mxmlAdd(returnvalue, MXML_ADD_AFTER, MXML_ADD_TO_PARENT, type);

		  description = mxmlNewElement(returnvalue, "description");
#ifdef DEBUG
		  fputs("    adding comment to returnvalue...\n", stderr);
#endif /* DEBUG */
		  update_comment(returnvalue, comment->last_child);
		  mxmlAdd(description, MXML_ADD_AFTER, MXML_ADD_TO_PARENT,
		          comment->last_child);
                }
		else
		  mxmlDelete(type);

		description = mxmlNewElement(function, "description");
#ifdef DEBUG
		  fputs("    adding comment to function...\n", stderr);
#endif /* DEBUG */
		update_comment(function, comment->last_child);
		mxmlAdd(description, MXML_ADD_AFTER, MXML_ADD_TO_PARENT,
		        comment->last_child);

		type = NULL;
	      }
	      else if (function && ((ch == ')' && parens == 1) || ch == ','))
	      {
	       /*
	        * Argument definition...
		*/

                if (strcmp(buffer, "void"))
		{
	          mxmlNewText(type, type->child != NULL &&
		                    type->last_child->value.text.string[0] != '(' &&
				    type->last_child->value.text.string[0] != '*',
			      buffer);

#ifdef DEBUG
                  fprintf(stderr, "Argument: <<<< %s >>>\n", buffer);
#endif /* DEBUG */

	          variable = add_variable(function, "argument", type);
		}
		else
		  mxmlDelete(type);

		type = NULL;
	      }
              else if (type->child && !function && (ch == ';' || ch == ','))
	      {
#ifdef DEBUG
	        fprintf(stderr, "    got semicolon, typedefnode=%p, structclass=%p\n",
		        typedefnode, structclass);
#endif /* DEBUG */

	        if (typedefnode || structclass)
		{
#ifdef DEBUG
                  fprintf(stderr, "Typedef/struct/class: <<<< %s >>>>\n", buffer);
#endif /* DEBUG */

		  if (typedefnode)
		  {
		    mxmlElementSetAttr(typedefnode, "name", buffer);

                    sort_node(tree, typedefnode);
		  }

		  if (structclass && !mxmlElementGetAttr(structclass, "name"))
		  {
#ifdef DEBUG
		    fprintf(stderr, "setting struct/class name to %s!\n",
		            type->last_child->value.text.string);
#endif /* DEBUG */
		    mxmlElementSetAttr(structclass, "name", buffer);

		    sort_node(tree, structclass);
		    structclass = NULL;
		  }

		  if (typedefnode)
		    mxmlAdd(typedefnode, MXML_ADD_BEFORE, MXML_ADD_TO_PARENT,
		            type);
                  else
		    mxmlDelete(type);

		  type        = NULL;
		  typedefnode = NULL;
		}
		else if (type->child &&
		         !strcmp(type->child->value.text.string, "typedef"))
		{
		 /*
		  * Simple typedef...
		  */

#ifdef DEBUG
                  fprintf(stderr, "Typedef: <<<< %s >>>\n", buffer);
#endif /* DEBUG */

		  typedefnode = mxmlNewElement(MXML_NO_PARENT, "typedef");
		  mxmlElementSetAttr(typedefnode, "name", buffer);
		  mxmlDelete(type->child);

                  sort_node(tree, typedefnode);

                  if (type->child)
		    type->child->value.text.whitespace = 0;

		  mxmlAdd(typedefnode, MXML_ADD_AFTER, MXML_ADD_TO_PARENT, type);
		  type = NULL;
		}
		else if (!parens)
		{
		 /*
	          * Variable definition...
		  */

	          if (type->child &&
		      !strcmp(type->child->value.text.string, "static") &&
		      !strcmp(tree->value.element.name, "mxmldoc"))
		  {
		   /*
		    * Remove static functions...
		    */

		    mxmlDelete(type);
		    type = NULL;
		    break;
		  }

	          mxmlNewText(type, type->child != NULL &&
		                    type->last_child->value.text.string[0] != '(' &&
				    type->last_child->value.text.string[0] != '*',
			      buffer);

#ifdef DEBUG
                  fprintf(stderr, "Variable: <<<< %s >>>>\n", buffer);
                  fprintf(stderr, "    scope = %s\n", scope ? scope : "(null)");
#endif /* DEBUG */

	          variable = add_variable(MXML_NO_PARENT, "variable", type);
		  type     = NULL;

		  sort_node(tree, variable);

		  if (scope)
		    mxmlElementSetAttr(variable, "scope", scope);
		}
              }
	      else
              {
#ifdef DEBUG
                fprintf(stderr, "Identifier: <<<< %s >>>>\n", buffer);
#endif /* DEBUG */

	        mxmlNewText(type, type->child != NULL &&
		                  type->last_child->value.text.string[0] != '(' &&
				  type->last_child->value.text.string[0] != '*',
			    buffer);
	      }
	    }
	    else if (enumeration && !isdigit(buffer[0] & 255))
	    {
#ifdef DEBUG
	      fprintf(stderr, "Constant: <<<< %s >>>\n", buffer);
#endif /* DEBUG */

	      constant = mxmlNewElement(MXML_NO_PARENT, "constant");
	      mxmlElementSetAttr(constant, "name", buffer);
	      sort_node(enumeration, constant);
	    }
	    else if (type)
	    {
	      mxmlDelete(type);
	      type = NULL;
	    }
	  }
          break;
    }

#if DEBUG > 1
    if (state != oldstate)
    {
      fprintf(stderr, "    changed states from %s to %s on receipt of character '%c'...\n",
              states[oldstate], states[state], oldch);
      fprintf(stderr, "    variable = %p\n", variable);
      if (type)
      {
        fputs("    type =", stderr);
        for (temp = type->child; temp; temp = temp->next)
	  fprintf(stderr, " \"%s\"", temp->value.text.string);
	fputs("\n", stderr);
      }
    }
#endif /* DEBUG > 1 */
  }

  mxmlDelete(comment);

 /*
  * All done, return with no errors...
  */

  return (0);
}


/*
 * 'sort_node()' - Insert a node sorted into a tree.
 */

static void
sort_node(mxml_node_t *tree,		/* I - Tree to sort into */
          mxml_node_t *node)		/* I - Node to add */
{
  mxml_node_t	*temp;			/* Current node */
  const char	*tempname,		/* Name of current node */
		*nodename,		/* Name of node */
		*scope;			/* Scope */


#if DEBUG > 1
  fprintf(stderr, "    sort_node(tree=%p, node=%p)\n", tree, node);
#endif /* DEBUG > 1 */

 /*
  * Range check input...
  */

  if (!tree || !node || node->parent == tree)
    return;

 /*
  * Get the node name...
  */

  if ((nodename = mxmlElementGetAttr(node, "name")) == NULL)
    return;

  if (nodename[0] == '_')
    return;				/* Hide private names */

#if DEBUG > 1
  fprintf(stderr, "        nodename=%p (\"%s\")\n", nodename, nodename);
#endif /* DEBUG > 1 */

 /*
  * Delete any existing definition at this level, if one exists...
  */

  if ((temp = mxmlFindElement(tree, tree, node->value.element.name,
                              "name", nodename, MXML_DESCEND_FIRST)) != NULL)
  {
   /*
    * Copy the scope if needed...
    */

    if ((scope = mxmlElementGetAttr(temp, "scope")) != NULL &&
        mxmlElementGetAttr(node, "scope") == NULL)
    {
#ifdef DEBUG
      fprintf(stderr, "    copying scope %s for %s\n", scope, nodename);
#endif /* DEBUG */

      mxmlElementSetAttr(node, "scope", scope);
    }

    mxmlDelete(temp);
  }

 /*
  * Add the node into the tree at the proper place...
  */

  for (temp = tree->child; temp; temp = temp->next)
  {
#if DEBUG > 1
    fprintf(stderr, "        temp=%p\n", temp);
#endif /* DEBUG > 1 */

    if ((tempname = mxmlElementGetAttr(temp, "name")) == NULL)
      continue;

#if DEBUG > 1
    fprintf(stderr, "        tempname=%p (\"%s\")\n", tempname, tempname);
#endif /* DEBUG > 1 */

    if (strcmp(nodename, tempname) < 0)
      break;
  }

  if (temp)
    mxmlAdd(tree, MXML_ADD_BEFORE, temp, node);
  else
    mxmlAdd(tree, MXML_ADD_AFTER, MXML_ADD_TO_PARENT, node);
}


/*
 * 'update_comment()' - Update a comment node.
 */

static void
update_comment(mxml_node_t *parent,	/* I - Parent node */
               mxml_node_t *comment)	/* I - Comment node */
{
  char	*ptr;				/* Pointer into comment */


#ifdef DEBUG
  fprintf(stderr, "update_comment(parent=%p, comment=%p)\n",
          parent, comment);
#endif /* DEBUG */

 /*
  * Range check the input...
  */

  if (!parent || !comment)
    return;
 
 /*
  * Update the comment...
  */

  ptr = comment->value.text.string;

  if (*ptr == '\'')
  {
   /*
    * Convert "'name()' - description" to "description".
    */

    for (ptr ++; *ptr && *ptr != '\''; ptr ++);

    if (*ptr == '\'')
    {
      ptr ++;
      while (isspace(*ptr & 255))
        ptr ++;

      if (*ptr == '-')
        ptr ++;

      while (isspace(*ptr & 255))
        ptr ++;

      safe_strcpy(comment->value.text.string, ptr);
    }
  }
  else if (!strncmp(ptr, "I ", 2) || !strncmp(ptr, "O ", 2) ||
           !strncmp(ptr, "IO ", 3))
  {
   /*
    * 'Convert "I - description", "IO - description", or "O - description"
    * to description + directory attribute.
    */

    ptr = strchr(ptr, ' ');
    *ptr++ = '\0';

    if (!strcmp(parent->value.element.name, "argument"))
      mxmlElementSetAttr(parent, "direction", comment->value.text.string);

    while (isspace(*ptr & 255))
      ptr ++;

    if (*ptr == '-')
      ptr ++;

    while (isspace(*ptr & 255))
      ptr ++;

    safe_strcpy(comment->value.text.string, ptr);
  }

 /*
  * Eliminate leading and trailing *'s...
  */

  for (ptr = comment->value.text.string; *ptr == '*'; ptr ++);
  for (; isspace(*ptr & 255); ptr ++);
  if (ptr > comment->value.text.string)
    safe_strcpy(comment->value.text.string, ptr);

  for (ptr = comment->value.text.string + strlen(comment->value.text.string) - 1;
       ptr > comment->value.text.string && *ptr == '*';
       ptr --)
    *ptr = '\0';
  for (; ptr > comment->value.text.string && isspace(*ptr & 255); ptr --)
    *ptr = '\0';

#ifdef DEBUG
  fprintf(stderr, "    updated comment = %s\n", comment->value.text.string);
#endif /* DEBUG */
}


/*
 * 'usage()' - Show program usage...
 */

static void
usage(const char *option)		/* I - Unknown option */
{
  if (option)
    printf("mxmldoc: Bad option \"%s\"!\n\n", option);

  puts("Usage: mxmldoc [options] [filename.xml] [source files] >filename.html");
  puts("Options:");
  puts("    --intro introfile          Set introduction file");
  puts("    --man name                 Generate man page");
  puts("    --no-output                Do no generate documentation file");
  puts("    --section section          Set section name");
  puts("    --title title              Set documentation title");

  exit(1);
}


/*
 * 'write_description()' - Write the description text.
 */

static void
write_description(
    mxml_node_t *description,		/* I - Description node */
    int         mode)			/* I - Output mode */
{
  char	text[10240],			/* Text for description */
	*ptr;				/* Pointer into text */
  int	col;				/* Current column */


  if (!description)
    return;

  get_text(description, text, sizeof(text));

  for (ptr = text, col = 0; *ptr; ptr ++)
  {
    if (*ptr == '@' &&
        (!strncmp(ptr + 1, "deprecated@", 11) ||
         !strncmp(ptr + 1, "since ", 6)))
    {
      ptr ++;
      while (*ptr && *ptr != '@')
        ptr ++;

      if (!*ptr)
        return;
    }
    else if (mode == OUTPUT_HTML)
    {
      if (*ptr == '&')
        fputs("&amp;", stdout);
      else if (*ptr == '<')
        fputs("&lt;", stdout);
      else if (*ptr == '>')
        fputs("&gt;", stdout);
      else if (*ptr == '\"')
        fputs("&quot;", stdout);
      else if (*ptr & 128)
      {
       /*
        * Convert UTF-8 to Unicode constant...
        */

        int	ch;			/* Unicode character */


        ch = *ptr & 255;

        if ((ch & 0xe0) == 0xc0)
        {
          ch = ((ch & 0x1f) << 6) | (ptr[1] & 0x3f);
	  ptr ++;
        }
        else if ((ch & 0xf0) == 0xe0)
        {
          ch = ((((ch * 0x0f) << 6) | (ptr[1] & 0x3f)) << 6) | (ptr[2] & 0x3f);
	  ptr += 2;
        }

        if (ch == 0xa0)
        {
         /*
          * Handle non-breaking space as-is...
	  */

          fputs("&nbsp;", stdout);
        }
        else
          printf("&#x%x;", ch);
      }
      else if (*ptr == '\n' && ptr[1] == '\n' && ptr[2] && ptr[2] != '@')
      {
        fputs("\n<p>", stdout);
        ptr ++;
      }
      else
        putchar(*ptr);
    }
    else if (*ptr == '\n' && ptr[1] == '\n' && ptr[2] && ptr[2] != '@')
    {
      puts("\n.PP");
      ptr ++;
    }
    else
    {
      if (*ptr == '\\' || (*ptr == '.' && col == 0))
        putchar('\\');

      putchar(*ptr);

      if (*ptr == '\n')
        col = 0;
      else
        col ++;
    }
  }

  putchar('\n');
}


/*
 * 'write_element()' - Write an element's text nodes.
 */

static void
write_element(mxml_node_t *doc,		/* I - Document tree */
              mxml_node_t *element,	/* I - Element to write */
              int         mode)		/* I - Output mode */
{
  mxml_node_t	*node;			/* Current node */


  if (!element)
    return;

  for (node = element->child;
       node;
       node = mxmlWalkNext(node, element, MXML_NO_DESCEND))
    if (node->type == MXML_TEXT)
    {
      if (node->value.text.whitespace)
	putchar(' ');

      if (mode == OUTPUT_HTML &&
          (mxmlFindElement(doc, doc, "class", "name", node->value.text.string,
                           MXML_DESCEND) ||
	   mxmlFindElement(doc, doc, "enumeration", "name",
	                   node->value.text.string, MXML_DESCEND) ||
	   mxmlFindElement(doc, doc, "struct", "name", node->value.text.string,
                           MXML_DESCEND) ||
	   mxmlFindElement(doc, doc, "typedef", "name", node->value.text.string,
                           MXML_DESCEND) ||
	   mxmlFindElement(doc, doc, "union", "name", node->value.text.string,
                           MXML_DESCEND)))
      {
        printf("<a href='#");
        write_string(node->value.text.string, mode);
	printf("'>");
        write_string(node->value.text.string, mode);
	printf("</a>");
      }
      else
        write_string(node->value.text.string, mode);
    }
}


/*
 * 'write_html()' - Write HTML documentation.
 */

static void
write_html(
    const char  *section,		/* I - Section */
    const char  *title,			/* I - Title */
    const char  *introfile,		/* I - Intro file */
    mxml_node_t *doc)			/* I - XML documentation */
{
  int		i;			/* Looping var */
  FILE		*fp;			/* File */
  char		line[8192];		/* Line from file */
  mxml_node_t	*function,		/* Current function */
		*scut,			/* Struct/class/union/typedef */
		*arg,			/* Current argument */
		*description,		/* Description of function/var */
		*type;			/* Type for argument */
  const char	*name,			/* Name of function/type */
		*cname,			/* Class name */
		*defval,		/* Default value */
		*parent;		/* Parent class */
  int		inscope;		/* Variable/method scope */
  char		prefix;			/* Prefix character */
  static const char * const scopes[] =	/* Scope strings */
		{
		  "private",
		  "protected",
		  "public"
		};


 /*
  * Standard header...
  */

  puts("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" "
       "\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n"
       "<html>");

  if (section)
    printf("<!-- SECTION: %s -->\n", section);

  printf("<head>\n"
	 "\t<title>%s</title>\n", title);
  if (section)
    printf("\t<meta name='keywords' content='%s'>\n", section);

  puts("\t<meta name='creator' content='" MXML_VERSION "'>\n"
       "\t<style type='text/css'><!--\n"
       "\th1, h2, h3, p { font-family: sans-serif; text-align: justify; }\n"
       "\ttt, pre a:link, pre a:visited, tt a:link, tt a:visited { font-weight: bold; color: #7f0000; }\n"
       "\tpre { font-weight: bold; color: #7f0000; margin-left: 2em; }\n"
       "\tspan.info { background: #000000; border: solid thin #000000; "
       "color: #ffffff; font-size: 80%; font-style: italic; "
       "font-weight: bold; white-space: nowrap; }\n"
       "\th3 span.info { float: right; font-size: 100%; }\n"
       "\th1.title, h2.title, h3.title { border-bottom: solid 2px #000000; }\n"
       "\t--></style>\n"
       "</head>\n"
       "<body>");

 /*
  * Intro...
  */

  if (introfile && (fp = fopen(introfile, "r")) != NULL)
  {
   /*
    * Insert intro file before contents...
    */

    while (fgets(line, sizeof(line), fp))
      fputs(line, stdout);

    fclose(fp);
  }

 /*
  * Table of contents...
  */

  puts("<h2 class='title'>Contents</h2>");
  puts("<ul>");
  if (find_public(doc, doc, "class"))
    puts("\t<li><a href='#CLASSES'>Classes</a></li>");
  if (find_public(doc, doc, "enumeration"))
    puts("\t<li><a href='#ENUMERATIONS'>Enumerations</a></li>");
  if (find_public(doc, doc, "function"))
    puts("\t<li><a href='#FUNCTIONS'>Functions</a></li>");
  if (find_public(doc, doc, "struct"))
    puts("\t<li><a href='#STRUCTURES'>Structures</a></li>");
  if (find_public(doc, doc, "typedef"))
    puts("\t<li><a href='#TYPES'>Types</a></li>");
  if (find_public(doc, doc, "union"))
    puts("\t<li><a href='#UNIONS'>Unions</a></li>");
  if (find_public(doc, doc, "variable"))
    puts("\t<li><a href='#VARIABLES'>Variables</a></li>");
  puts("</ul>");

 /*
  * List of classes...
  */

  if (find_public(doc, doc, "class"))
  {
    puts("<!-- NEW PAGE -->\n"
         "<h2 class='title'><a name='CLASSES'>Classes</a></h2>\n"
         "<ul>");

    for (scut = find_public(doc, doc, "class");
	 scut;
	 scut = find_public(scut, doc, "class"))
    {
      name = mxmlElementGetAttr(scut, "name");
      printf("\t<li><a href='#%s'><tt>%s</tt></a> %s</li>\n", name, name,
             get_comment_info(mxmlFindElement(scut, scut, "description",
	                                      NULL, NULL, MXML_DESCEND_FIRST)));
    }

    puts("</ul>");

    for (scut = find_public(doc, doc, "class");
	 scut;
	 scut = find_public(scut, doc, "class"))
    {
      cname       = mxmlElementGetAttr(scut, "name");
      description = mxmlFindElement(scut, scut, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf("<!-- NEW PAGE -->\n"
             "<h3 class='title'>%s<a name='%s'>%s</a></h3>\n",
	     get_comment_info(description), cname, cname);

      if (description)
      {
        fputs("<h4>Description</h4>\n"
	      "<p>", stdout);
	write_description(description, OUTPUT_HTML);
      }

      printf("<h4>Definition</h4>\n"
             "<p><tt>\n"
             "class %s", cname);
      if ((parent = mxmlElementGetAttr(scut, "parent")) != NULL)
        printf(" %s", parent);
      puts("<br>\n{");

      for (i = 0; i < 3; i ++)
      {
        inscope = 0;

	for (arg = mxmlFindElement(scut, scut, "variable", "scope", scopes[i],
                        	   MXML_DESCEND_FIRST);
	     arg;
	     arg = mxmlFindElement(arg, scut, "variable", "scope", scopes[i],
                        	   MXML_NO_DESCEND))
	{
          if (!inscope)
	  {
	    inscope = 1;
	    printf("&nbsp;&nbsp;%s:<br>\n", scopes[i]);
	  }

	  printf("&nbsp;&nbsp;&nbsp;&nbsp;");
	  write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                             NULL, MXML_DESCEND_FIRST),
                        OUTPUT_HTML);
	  printf(" %s;<br>\n", mxmlElementGetAttr(arg, "name"));
	}

	for (function = mxmlFindElement(scut, scut, "function", "scope",
	                                scopes[i], MXML_DESCEND_FIRST);
	     function;
	     function = mxmlFindElement(function, scut, "function", "scope",
	                                scopes[i], MXML_NO_DESCEND))
	{
          if (!inscope)
	  {
	    inscope = 1;
	    printf("&nbsp;&nbsp;%s:<br>\n", scopes[i]);
	  }

          name = mxmlElementGetAttr(function, "name");

          printf("&nbsp;&nbsp;&nbsp;&nbsp;");

	  arg = mxmlFindElement(function, function, "returnvalue", NULL,
                        	NULL, MXML_DESCEND_FIRST);

	  if (arg)
	  {
	    write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                               NULL, MXML_DESCEND_FIRST),
                          OUTPUT_HTML);
	    putchar(' ');
	  }
	  else if (strcmp(cname, name) && strcmp(cname, name + 1))
	    fputs("void ", stdout);

	  printf("<a href='#%s.%s'>%s</a>", cname, name, name);

	  for (arg = mxmlFindElement(function, function, "argument", NULL, NULL,
                        	     MXML_DESCEND_FIRST), prefix = '(';
	       arg;
	       arg = mxmlFindElement(arg, function, "argument", NULL, NULL,
                        	     MXML_NO_DESCEND), prefix = ',')
	  {
	    type = mxmlFindElement(arg, arg, "type", NULL, NULL,
	                	   MXML_DESCEND_FIRST);

	    putchar(prefix);
	    if (prefix == ',')
	      putchar(' ');

	    if (type->child)
	    {
	      write_element(doc, type, OUTPUT_HTML);
	      putchar(' ');
	    }
	    fputs(mxmlElementGetAttr(arg, "name"), stdout);
            if ((defval = mxmlElementGetAttr(arg, "default")) != NULL)
	      printf(" %s", defval);
	  }

	  if (prefix == '(')
	    puts("(void);<br>");
	  else
	    puts(");<br>");
	}
      }

      puts("};</tt></p>\n"
           "<h4>Members</h4>\n"
           "<div class='table'><table align='center' border='1' width='80%' "
           "summary='Members'>\n"
           "<thead><tr><th>Name</th><th>Description</th></tr></thead>\n"
           "<tbody>");

      for (arg = mxmlFindElement(scut, scut, "variable", NULL, NULL,
                        	 MXML_DESCEND_FIRST);
	   arg;
	   arg = mxmlFindElement(arg, scut, "variable", NULL, NULL,
                        	 MXML_NO_DESCEND))
      {
        description = mxmlFindElement(arg, arg, "description", NULL,
                                      NULL, MXML_DESCEND_FIRST);

	printf("<tr><td><tt>%s</tt> %s</td><td>",
	       mxmlElementGetAttr(arg, "name"), get_comment_info(description));

	write_description(description, OUTPUT_HTML);

	puts("</td></tr>");
      }

      for (function = mxmlFindElement(scut, scut, "function", NULL, NULL,
                                      MXML_DESCEND_FIRST);
	   function;
	   function = mxmlFindElement(function, scut, "function", NULL, NULL,
                                      MXML_NO_DESCEND))
      {
	name        = mxmlElementGetAttr(function, "name");
	description = mxmlFindElement(function, function, "description", NULL,
                                      NULL, MXML_DESCEND_FIRST);

	printf("<tr><td><tt><a name='%s.%s'>%s()</a></tt> %s</td><td>",
	       cname, name, name, get_comment_info(description));

	write_description(description, OUTPUT_HTML);

	arg = mxmlFindElement(function, function, "returnvalue", NULL,
                              NULL, MXML_DESCEND_FIRST);

	if (arg)
	{
	  fputs("\n<i>Returns:</i> ", stdout);
	  write_element(NULL, mxmlFindElement(arg, arg, "description", NULL,
                                              NULL, MXML_DESCEND_FIRST),
                        OUTPUT_HTML);
	}

	puts("</td></tr>");
      }

      puts("</tbody></table></div>");
    }
  }

 /*
  * List of enumerations...
  */

  if (find_public(doc, doc, "enumeration"))
  {
    puts("<!-- NEW PAGE -->\n"
         "<h2 class='title'><a name='ENUMERATIONS'>Enumerations</a></h2>\n"
         "<ul>");

    for (scut = find_public(doc, doc, "enumeration");
	 scut;
	 scut = find_public(scut, doc, "enumeration"))
    {
      name = mxmlElementGetAttr(scut, "name");
      printf("\t<li><a href='#%s'><tt>%s</tt></a> %s</li>\n", name, name,
             get_comment_info(mxmlFindElement(scut, scut, "description",
	                                      NULL, NULL, MXML_DESCEND_FIRST)));
    }

    puts("</ul>");

    for (scut = find_public(doc, doc, "enumeration");
	 scut;
	 scut = find_public(scut, doc, "enumeration"))
    {
      name        = mxmlElementGetAttr(scut, "name");
      description = mxmlFindElement(scut, scut, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf("<!-- NEW PAGE -->\n"
             "<h3 class='title'>%s<a name='%s'>%s</a></h3>\n",
             get_comment_info(description), name, name);

      if (description)
      {
        fputs("<h4>Description</h4>\n"
	      "<p>", stdout);
	write_description(description, OUTPUT_HTML);
      }

      puts("<h4>Values</h4>\n"
           "<div class='table'><table align='center' border='1' width='80%' "
           "summary='Values'>\n"
           "<thead><tr><th>Name</th><th>Description</th></tr></thead>\n"
           "<tbody>");

      for (arg = mxmlFindElement(scut, scut, "constant", NULL, NULL,
                        	 MXML_DESCEND_FIRST);
	   arg;
	   arg = mxmlFindElement(arg, scut, "constant", NULL, NULL,
                        	 MXML_NO_DESCEND))
      {
	description = mxmlFindElement(arg, arg, "description", NULL,
                                      NULL, MXML_DESCEND_FIRST);
	printf("<tr><td><tt>%s</tt> %s</td><td>",
	       mxmlElementGetAttr(arg, "name"), get_comment_info(description));

	write_description(description, OUTPUT_HTML);

	puts("</td></tr>");
      }

      puts("</tbody></table></div>");
    }
  }

 /*
  * List of functions...
  */

  if (find_public(doc, doc, "function"))
  {
    puts("<!-- NEW PAGE -->\n"
         "<h2 class='title'><a name='FUNCTIONS'>Functions</a></h2>\n"
         "<ul>");

    for (function = find_public(doc, doc, "function");
	 function;
	 function = find_public(function, doc, "function"))
    {
      name = mxmlElementGetAttr(function, "name");
      printf("\t<li><a href='#%s'><tt>%s()</tt></a> %s</li>\n", name, name,
             get_comment_info(mxmlFindElement(function, function, "description",
	                                      NULL, NULL, MXML_DESCEND_FIRST)));
    }

    puts("</ul>");

    for (function = find_public(doc, doc, "function");
	 function;
	 function = find_public(function, doc, "function"))
    {
      name        = mxmlElementGetAttr(function, "name");
      description = mxmlFindElement(function, function, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf("<!-- NEW PAGE -->\n"
             "<h3 class='title'>%s<a name='%s'>%s()</a></h3>\n",
             get_comment_info(description), name, name);

      if (description)
      {
        fputs("<h4>Description</h4>\n"
	      "<p>", stdout);
	write_description(description, OUTPUT_HTML);
      }

      puts("<h4>Syntax</h4>\n"
           "<p><tt>");

      arg = mxmlFindElement(function, function, "returnvalue", NULL,
                            NULL, MXML_DESCEND_FIRST);

      if (arg)
	write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                           NULL, MXML_DESCEND_FIRST),
                      OUTPUT_HTML);
      else
	fputs("void", stdout);

      printf("<br>\n%s", name);
      for (arg = mxmlFindElement(function, function, "argument", NULL, NULL,
                        	 MXML_DESCEND_FIRST), prefix = '(';
	   arg;
	   arg = mxmlFindElement(arg, function, "argument", NULL, NULL,
                        	 MXML_NO_DESCEND), prefix = ',')
      {
        type = mxmlFindElement(arg, arg, "type", NULL, NULL,
	                       MXML_DESCEND_FIRST);

	printf("%c\n    ", prefix);
	if (type->child)
	{
	  write_element(doc, type, OUTPUT_HTML);
	  putchar(' ');
	}
	fputs(mxmlElementGetAttr(arg, "name"), stdout);
        if ((defval = mxmlElementGetAttr(arg, "default")) != NULL)
	  printf(" %s", defval);
      }

      if (prefix == '(')
	puts("(void);\n</tt></p>");
      else
	puts(");\n</tt></p>");

      puts("<h4>Arguments</h4>");

      if (prefix == '(')
	puts("<p>None.</p>");
      else
      {
	puts("<div class='table'><table align='center' border='1' width='80%' "
             "cellpadding='5' cellspacing='0' summary='Arguments'>\n"
	     "<thead><tr><th>Name</th><th>Description</th></tr></thead>\n"
	     "<tbody>");

	for (arg = mxmlFindElement(function, function, "argument", NULL, NULL,
                        	   MXML_DESCEND_FIRST);
	     arg;
	     arg = mxmlFindElement(arg, function, "argument", NULL, NULL,
                        	   MXML_NO_DESCEND))
	{
	  printf("<tr><td><tt>%s</tt></td><td>", mxmlElementGetAttr(arg, "name"));

	  write_description(mxmlFindElement(arg, arg, "description", NULL,
                               		    NULL, MXML_DESCEND_FIRST),
                            OUTPUT_HTML);

          puts("</td></tr>");
	}

	puts("</tbody></table></div>");
      }

      puts("<h4>Returns</h4>");

      arg = mxmlFindElement(function, function, "returnvalue", NULL,
                            NULL, MXML_DESCEND_FIRST);

      if (!arg)
	puts("<p>Nothing.</p>");
      else
      {
	fputs("<p>", stdout);
	write_element(NULL, mxmlFindElement(arg, arg, "description", NULL,
                                            NULL, MXML_DESCEND_FIRST),
                      OUTPUT_HTML);
	puts("</p>");
      }
    }
  }

 /*
  * List of structures...
  */

  if (find_public(doc, doc, "struct"))
  {
    puts("<!-- NEW PAGE -->\n"
         "<h2 class='title'><a name='STRUCTURES'>Structures</a></h2>\n"
         "<ul>");

    for (scut = find_public(doc, doc, "struct");
	 scut;
	 scut = find_public(scut, doc, "struct"))
    {
      name = mxmlElementGetAttr(scut, "name");
      printf("\t<li><a href='#%s'><tt>%s</tt></a> %s</li>\n", name, name,
             get_comment_info(mxmlFindElement(scut, scut, "description",
	                                      NULL, NULL, MXML_DESCEND_FIRST)));
    }

    puts("</ul>");

    for (scut = find_public(doc, doc, "struct");
	 scut;
	 scut = find_public(scut, doc, "struct"))
    {
      cname       = mxmlElementGetAttr(scut, "name");
      description = mxmlFindElement(scut, scut, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf("<!-- NEW PAGE -->\n"
             "<h3 class='title'>%s<a name='%s'>%s</a></h3>\n",
	     get_comment_info(description), cname, cname);

      if (description)
      {
        fputs("<h4>Description</h4>\n"
	      "<p>", stdout);
	write_description(description, OUTPUT_HTML);
      }

      printf("<h4>Definition</h4>\n"
             "<p><tt>\n"
	     "struct %s<br>\n{<br>\n", cname);
      for (arg = mxmlFindElement(scut, scut, "variable", NULL, NULL,
                        	 MXML_DESCEND_FIRST);
	   arg;
	   arg = mxmlFindElement(arg, scut, "variable", NULL, NULL,
                        	 MXML_NO_DESCEND))
      {
	printf("&nbsp;&nbsp;");
	write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                           NULL, MXML_DESCEND_FIRST),
                      OUTPUT_HTML);
	printf(" %s;<br>\n", mxmlElementGetAttr(arg, "name"));
      }

      for (function = mxmlFindElement(scut, scut, "function", NULL, NULL,
                                      MXML_DESCEND_FIRST);
	   function;
	   function = mxmlFindElement(function, scut, "function", NULL, NULL,
                                      MXML_NO_DESCEND))
      {
        name = mxmlElementGetAttr(function, "name");

        printf("&nbsp;&nbsp;");

	arg = mxmlFindElement(function, function, "returnvalue", NULL,
                              NULL, MXML_DESCEND_FIRST);

	if (arg)
	{
	  write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                             NULL, MXML_DESCEND_FIRST),
                        OUTPUT_HTML);
	  putchar(' ');
	}
	else if (strcmp(cname, name) && strcmp(cname, name + 1))
	  fputs("void ", stdout);

	printf("<a href='#%s.%s'>%s</a>", cname, name, name);

	for (arg = mxmlFindElement(function, function, "argument", NULL, NULL,
                        	   MXML_DESCEND_FIRST), prefix = '(';
	     arg;
	     arg = mxmlFindElement(arg, function, "argument", NULL, NULL,
                        	   MXML_NO_DESCEND), prefix = ',')
	{
	  type = mxmlFindElement(arg, arg, "type", NULL, NULL,
	                	 MXML_DESCEND_FIRST);

	  putchar(prefix);
	  if (prefix == ',')
	    putchar(' ');

	  if (type->child)
	  {
	    write_element(doc, type, OUTPUT_HTML);
	    putchar(' ');
	  }
	  fputs(mxmlElementGetAttr(arg, "name"), stdout);
          if ((defval = mxmlElementGetAttr(arg, "default")) != NULL)
	    printf(" %s", defval);
	}

	if (prefix == '(')
	  puts("(void);<br>");
	else
	  puts(");<br>");
      }

      puts("};</tt></p>\n"
           "<h4>Members</h4>\n"
           "<div class='table'><table align='center' border='1' width='80%' "
           "summary='Members'>\n"
           "<thead><tr><th>Name</th><th>Description</th></tr></thead>\n"
           "<tbody>");

      for (arg = mxmlFindElement(scut, scut, "variable", NULL, NULL,
                        	 MXML_DESCEND_FIRST);
	   arg;
	   arg = mxmlFindElement(arg, scut, "variable", NULL, NULL,
                        	 MXML_NO_DESCEND))
      {
        description = mxmlFindElement(arg, arg, "description", NULL,
                                      NULL, MXML_DESCEND_FIRST);
	printf("<tr><td><tt>%s</tt> %s</td><td>",
	       mxmlElementGetAttr(arg, "name"), get_comment_info(description));

	write_description(description, OUTPUT_HTML);

	puts("</td></tr>");
      }

      for (function = mxmlFindElement(scut, scut, "function", NULL, NULL,
                                      MXML_DESCEND_FIRST);
	   function;
	   function = mxmlFindElement(function, scut, "function", NULL, NULL,
                                      MXML_NO_DESCEND))
      {
	name        = mxmlElementGetAttr(function, "name");
	description = mxmlFindElement(function, function, "description", NULL,
                                      NULL, MXML_DESCEND_FIRST);

	printf("<tr><td><tt><a name='%s.%s'>%s()</a></tt> %s</td><td>",
	       cname, name, name, get_comment_info(description));

	write_description(description, OUTPUT_HTML);

	arg = mxmlFindElement(function, function, "returnvalue", NULL,
                              NULL, MXML_DESCEND_FIRST);

	if (arg)
	{
	  fputs("\n<i>Returns:</i> ", stdout);
	  write_element(NULL, mxmlFindElement(arg, arg, "description", NULL,
                                              NULL, MXML_DESCEND_FIRST),
                        OUTPUT_HTML);
	}

	puts("</td></tr>");
      }

      puts("</tbody></table></div>");
    }
  }

 /*
  * List of types...
  */

  if (find_public(doc, doc, "typedef"))
  {
    puts("<!-- NEW PAGE -->\n"
         "<h2 class='title'><a name='TYPES'>Types</a></h2>\n"
         "<ul>");

    for (scut = find_public(doc, doc, "typedef");
	 scut;
	 scut = find_public(scut, doc, "typedef"))
    {
      name = mxmlElementGetAttr(scut, "name");
      printf("\t<li><a href='#%s'><tt>%s</tt></a> %s</li>\n", name, name,
             get_comment_info(mxmlFindElement(scut, scut, "description",
	                                      NULL, NULL, MXML_DESCEND_FIRST)));
    }

    puts("</ul>");

    for (scut = find_public(doc, doc, "typedef");
	 scut;
	 scut = find_public(scut, doc, "typedef"))
    {
      name        = mxmlElementGetAttr(scut, "name");
      description = mxmlFindElement(scut, scut, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf("<!-- NEW PAGE -->\n"
             "<h3 class='title'>%s<a name='%s'>%s</a></h3>\n",
	     get_comment_info(description), name, name);

      if (description)
      {
        fputs("<h4>Description</h4>\n"
	      "<p>", stdout);
	write_description(description, OUTPUT_HTML);
      }

      fputs("<h4>Definition</h4>\n"
            "<p><tt>\n"
	    "typedef ", stdout);

      type = mxmlFindElement(scut, scut, "type", NULL, NULL,
                             MXML_DESCEND_FIRST);

      for (type = type->child; type; type = type->next)
        if (!strcmp(type->value.text.string, "("))
	  break;
	else
	{
	  if (type->value.text.whitespace)
	    putchar(' ');

	  if (mxmlFindElement(doc, doc, "class", "name",
	                      type->value.text.string, MXML_DESCEND) ||
	      mxmlFindElement(doc, doc, "enumeration", "name",
	                      type->value.text.string, MXML_DESCEND) ||
	      mxmlFindElement(doc, doc, "struct", "name",
	                      type->value.text.string, MXML_DESCEND) ||
	      mxmlFindElement(doc, doc, "typedef", "name",
	                      type->value.text.string, MXML_DESCEND) ||
	      mxmlFindElement(doc, doc, "union", "name",
	                      type->value.text.string, MXML_DESCEND))
	  {
            printf("<a href='#");
            write_string(type->value.text.string, OUTPUT_HTML);
	    printf("'>");
            write_string(type->value.text.string, OUTPUT_HTML);
	    printf("</a>");
	  }
	  else
            write_string(type->value.text.string, OUTPUT_HTML);
        }

      if (type)
      {
       /*
        * Output function type...
	*/

        printf(" (*%s", name);

	for (type = type->next->next; type; type = type->next)
	{
	  if (type->value.text.whitespace)
	    putchar(' ');

	  if (mxmlFindElement(doc, doc, "class", "name",
	                      type->value.text.string, MXML_DESCEND) ||
	      mxmlFindElement(doc, doc, "enumeration", "name",
	                      type->value.text.string, MXML_DESCEND) ||
	      mxmlFindElement(doc, doc, "struct", "name",
	                      type->value.text.string, MXML_DESCEND) ||
	      mxmlFindElement(doc, doc, "typedef", "name",
	                      type->value.text.string, MXML_DESCEND) ||
	      mxmlFindElement(doc, doc, "union", "name",
	                      type->value.text.string, MXML_DESCEND))
	  {
            printf("<a href='#");
            write_string(type->value.text.string, OUTPUT_HTML);
	    printf("'>");
            write_string(type->value.text.string, OUTPUT_HTML);
	    printf("</a>");
	  }
	  else
            write_string(type->value.text.string, OUTPUT_HTML);
        }

        puts(";");
      }
      else
	printf(" %s;\n", name);

      puts("</tt></p>");
    }
  }

 /*
  * List of unions...
  */

  if (find_public(doc, doc, "union"))
  {
    puts("<!-- NEW PAGE -->\n"
         "<h2 class='title'><a name='UNIONS'>Unions</a></h2>\n"
         "<ul>");

    for (scut = find_public(doc, doc, "union");
	 scut;
	 scut = find_public(scut, doc, "union"))
    {
      name = mxmlElementGetAttr(scut, "name");
      printf("\t<li><a href='#%s'><tt>%s</tt></a> %s</li>\n", name, name,
             get_comment_info(mxmlFindElement(scut, scut, "description",
	                                      NULL, NULL, MXML_DESCEND_FIRST)));
    }

    puts("</ul>");

    for (scut = find_public(doc, doc, "union");
	 scut;
	 scut = find_public(scut, doc, "union"))
    {
      name        = mxmlElementGetAttr(scut, "name");
      description = mxmlFindElement(scut, scut, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf("<!-- NEW PAGE -->\n"
             "<h3 class='title'>%s<a name='%s'>%s</a></h3>\n",
	     get_comment_info(description), name, name);

      if (description)
      {
        fputs("<h4>Description</h4>\n"
	      "<p>", stdout);
	write_description(description, OUTPUT_HTML);
      }

      printf("<h4>Definition</h4>\n"
             "<p><tt>\n"
	     "union %s<br>\n{<br>\n", name);
      for (arg = mxmlFindElement(scut, scut, "variable", NULL, NULL,
                        	 MXML_DESCEND_FIRST);
	   arg;
	   arg = mxmlFindElement(arg, scut, "variable", NULL, NULL,
                        	 MXML_NO_DESCEND))
      {
	printf("&nbsp;&nbsp;");
	write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                           NULL, MXML_DESCEND_FIRST),
                      OUTPUT_HTML);
	printf(" %s;<br>\n", mxmlElementGetAttr(arg, "name"));
      }

      puts("};</tt></p>\n"
           "<h4>Members</h4>\n"
           "<div class='table'><table align='center' border='1' width='80%' "
           "summary='Members'>\n"
           "<thead><tr><th>Name</th><th>Description</th></tr></thead>\n"
           "<tbody>");

      for (arg = mxmlFindElement(scut, scut, "variable", NULL, NULL,
                        	 MXML_DESCEND_FIRST);
	   arg;
	   arg = mxmlFindElement(arg, scut, "variable", NULL, NULL,
                        	 MXML_NO_DESCEND))
      {
        description = mxmlFindElement(arg, arg, "description", NULL,
                                      NULL, MXML_DESCEND_FIRST);
	printf("<tr><td><tt>%s</tt> %s</td><td>",
	       mxmlElementGetAttr(arg, "name"), get_comment_info(description));

	write_description(description, OUTPUT_HTML);

	puts("</td></tr>");
      }

      puts("</tbody></table></div>");
    }
  }

 /*
  * Variables...
  */

  if (find_public(doc, doc, "variable"))
  {
    puts("<!-- NEW PAGE -->\n"
         "<h2 class='title'><a name='VARIABLES'>Variables</a></h2>\n"
         "<ul>");

    for (arg = find_public(doc, doc, "variable");
	 arg;
	 arg = find_public(arg, doc, "variable"))
    {
      name = mxmlElementGetAttr(arg, "name");
      printf("\t<li><a href='#%s'><tt>%s</tt></a> %s</li>\n", name, name,
             get_comment_info(mxmlFindElement(arg, arg, "description",
	                                      NULL, NULL, MXML_DESCEND_FIRST)));
    }

    puts("</ul>");

    for (arg = find_public(doc, doc, "variable");
	 arg;
	 arg = find_public(arg, doc, "variable"))
    {
      name        = mxmlElementGetAttr(arg, "name");
      description = mxmlFindElement(arg, arg, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf("<!-- NEW PAGE -->\n"
             "<h3 class='title'>%s<a name='%s'>%s</a></h3>\n",
	     get_comment_info(description), name, name);

      if (description)
      {
        fputs("<h4>Description</h4>\n"
	      "<p>", stdout);
	write_description(description, OUTPUT_HTML);
      }

      puts("<h4>Definition</h4>\n"
           "<p><tt>");

      write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                         NULL, MXML_DESCEND_FIRST),
                    OUTPUT_HTML);
      printf(" %s", mxmlElementGetAttr(arg, "name"));
      if ((defval = mxmlElementGetAttr(arg, "default")) != NULL)
	printf(" %s", defval);
      puts(";</tt></p>");
    }
  }

 /*
  * Standard footer...
  */

  puts("</body>\n"
       "</html>");
}


/*
 * 'write_man()' - Write manpage documentation.
 */

static void
write_man(
    const char  *man_name,		/* I - Name of manpage */
    const char  *section,		/* I - Section */
    const char  *title,			/* I - Title */
    const char  *introfile,		/* I - Intro file */
    mxml_node_t *doc)			/* I - XML documentation */
{
  int		i;			/* Looping var */
  FILE		*fp;			/* File */
  char		line[8192];		/* Line from file */
  mxml_node_t	*function,		/* Current function */
		*scut,			/* Struct/class/union/typedef */
		*arg,			/* Current argument */
		*description,		/* Description of function/var */
		*type;			/* Type for argument */
  const char	*name,			/* Name of function/type */
		*cname,			/* Class name */
		*defval,		/* Default value */
		*parent;		/* Parent class */
  int		inscope;		/* Variable/method scope */
  char		prefix;			/* Prefix character */
  time_t	curtime;		/* Current time */
  struct tm	*curdate;		/* Current date */
  static const char * const scopes[] =	/* Scope strings */
		{
		  "private",
		  "protected",
		  "public"
		};


 /*
  * Standard header...
  */

  curtime = time(NULL);
  curdate = localtime(&curtime);
  strftime(line, sizeof(line), "%x", curdate);

  printf(".TH %s %s \"%s\" \"%s\" \"%s\"\n", man_name, section ? section : "3",
         title ? title : "", line, title ? title : "");
  puts(".SH NAME");
  printf("%s \\- %s\n", man_name, title ? title : man_name);

 /*
  * Intro...
  */

  if (introfile && (fp = fopen(introfile, "r")) != NULL)
  {
   /*
    * Insert intro file before contents...
    */

    while (fgets(line, sizeof(line), fp))
      fputs(line, stdout);

    fclose(fp);
  }

 /*
  * List of classes...
  */

  if (find_public(doc, doc, "class"))
  {
    puts(".SH CLASSES");

    for (scut = find_public(doc, doc, "class");
	 scut;
	 scut = find_public(scut, doc, "class"))
    {
      cname       = mxmlElementGetAttr(scut, "name");
      description = mxmlFindElement(scut, scut, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf(".SS %s\n", cname);

      printf(".nf\n"
             "class %s", cname);
      if ((parent = mxmlElementGetAttr(scut, "parent")) != NULL)
        printf(" %s", parent);
      puts("\n{");

      for (i = 0; i < 3; i ++)
      {
        inscope = 0;

	for (arg = mxmlFindElement(scut, scut, "variable", "scope", scopes[i],
                        	   MXML_DESCEND_FIRST);
	     arg;
	     arg = mxmlFindElement(arg, scut, "variable", "scope", scopes[i],
                        	   MXML_NO_DESCEND))
	{
          if (!inscope)
	  {
	    inscope = 1;
	    printf("  %s:\n", scopes[i]);
	  }

	  printf("    ");
	  write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                             NULL, MXML_DESCEND_FIRST),
                        OUTPUT_MAN);
	  printf(" %s;\n", mxmlElementGetAttr(arg, "name"));
	}

	for (function = mxmlFindElement(scut, scut, "function", "scope",
	                                scopes[i], MXML_DESCEND_FIRST);
	     function;
	     function = mxmlFindElement(function, scut, "function", "scope",
	                                scopes[i], MXML_NO_DESCEND))
	{
          if (!inscope)
	  {
	    inscope = 1;
	    printf("  %s:\n", scopes[i]);
	  }

          name = mxmlElementGetAttr(function, "name");

          printf("    ");

	  arg = mxmlFindElement(function, function, "returnvalue", NULL,
                        	NULL, MXML_DESCEND_FIRST);

	  if (arg)
	  {
	    write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                               NULL, MXML_DESCEND_FIRST),
                          OUTPUT_MAN);
	    putchar(' ');
	  }
	  else if (strcmp(cname, name) && strcmp(cname, name + 1))
	    fputs("void ", stdout);

	  printf("%s", name);

	  for (arg = mxmlFindElement(function, function, "argument", NULL, NULL,
                        	     MXML_DESCEND_FIRST), prefix = '(';
	       arg;
	       arg = mxmlFindElement(arg, function, "argument", NULL, NULL,
                        	     MXML_NO_DESCEND), prefix = ',')
	  {
	    type = mxmlFindElement(arg, arg, "type", NULL, NULL,
	                	   MXML_DESCEND_FIRST);

	    putchar(prefix);
	    if (prefix == ',')
	      putchar(' ');

	    if (type->child)
	    {
	      write_element(doc, type, OUTPUT_MAN);
	      putchar(' ');
	    }
	    fputs(mxmlElementGetAttr(arg, "name"), stdout);
            if ((defval = mxmlElementGetAttr(arg, "default")) != NULL)
	      printf(" %s", defval);
	  }

	  if (prefix == '(')
	    puts("(void);");
	  else
	    puts(");");
	}
      }

      puts("};\n"
           ".fi\n"
           ".PP");

      if (description)
	write_description(description, OUTPUT_MAN);
    }
  }

 /*
  * List of enumerations...
  */

  if (find_public(doc, doc, "enumeration"))
  {
    puts(".SH ENUMERATIONS");

    for (scut = find_public(doc, doc, "enumeration");
	 scut;
	 scut = find_public(scut, doc, "enumeration"))
    {
      name        = mxmlElementGetAttr(scut, "name");
      description = mxmlFindElement(scut, scut, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf(".SS %s\n", name);

      if (description)
	write_description(description, OUTPUT_MAN);

      for (arg = mxmlFindElement(scut, scut, "constant", NULL, NULL,
                        	 MXML_DESCEND_FIRST);
	   arg;
	   arg = mxmlFindElement(arg, scut, "constant", NULL, NULL,
                        	 MXML_NO_DESCEND))
      {
	description = mxmlFindElement(arg, arg, "description", NULL,
                                      NULL, MXML_DESCEND_FIRST);
	printf(".TP 5\n%s\n.br\n", mxmlElementGetAttr(arg, "name"));
	write_description(description, OUTPUT_MAN);
      }
    }
  }

 /*
  * List of functions...
  */

  if (find_public(doc, doc, "function"))
  {
    puts(".SH FUNCTIONS");

    for (function = find_public(doc, doc, "function");
	 function;
	 function = find_public(function, doc, "function"))
    {
      name        = mxmlElementGetAttr(function, "name");
      description = mxmlFindElement(function, function, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf(".SS %s()\n", name);

      puts(".nf");

      arg = mxmlFindElement(function, function, "returnvalue", NULL,
                            NULL, MXML_DESCEND_FIRST);

      if (arg)
	write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                           NULL, MXML_DESCEND_FIRST),
                      OUTPUT_MAN);
      else
	fputs("void", stdout);

      printf("\n%s", name);
      for (arg = mxmlFindElement(function, function, "argument", NULL, NULL,
                        	 MXML_DESCEND_FIRST), prefix = '(';
	   arg;
	   arg = mxmlFindElement(arg, function, "argument", NULL, NULL,
                        	 MXML_NO_DESCEND), prefix = ',')
      {
        type = mxmlFindElement(arg, arg, "type", NULL, NULL,
	                       MXML_DESCEND_FIRST);

	printf("%c\n    ", prefix);
	if (type->child)
	{
	  write_element(doc, type, OUTPUT_MAN);
	  putchar(' ');
	}
	fputs(mxmlElementGetAttr(arg, "name"), stdout);
        if ((defval = mxmlElementGetAttr(arg, "default")) != NULL)
	  printf(" %s", defval);
      }

      if (prefix == '(')
	puts("(void);");
      else
	puts(");");

      puts(".fi\n.PP");

      if (description)
	write_description(description, OUTPUT_MAN);
    }
  }

 /*
  * List of structures...
  */

  if (find_public(doc, doc, "struct"))
  {
    puts(".SH STRUCTURES");

    for (scut = find_public(doc, doc, "struct");
	 scut;
	 scut = find_public(scut, doc, "struct"))
    {
      cname       = mxmlElementGetAttr(scut, "name");
      description = mxmlFindElement(scut, scut, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf(".SS %s\n", cname);

      printf(".nf\n"
	     "struct %s\n{\n", cname);
      for (arg = mxmlFindElement(scut, scut, "variable", NULL, NULL,
                        	 MXML_DESCEND_FIRST);
	   arg;
	   arg = mxmlFindElement(arg, scut, "variable", NULL, NULL,
                        	 MXML_NO_DESCEND))
      {
	printf("  ");
	write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                           NULL, MXML_DESCEND_FIRST),
                      OUTPUT_MAN);
	printf(" %s;\n", mxmlElementGetAttr(arg, "name"));
      }

      for (function = mxmlFindElement(scut, scut, "function", NULL, NULL,
                                      MXML_DESCEND_FIRST);
	   function;
	   function = mxmlFindElement(function, scut, "function", NULL, NULL,
                                      MXML_NO_DESCEND))
      {
        name = mxmlElementGetAttr(function, "name");

        printf("  ");

	arg = mxmlFindElement(function, function, "returnvalue", NULL,
                              NULL, MXML_DESCEND_FIRST);

	if (arg)
	{
	  write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                             NULL, MXML_DESCEND_FIRST),
                        OUTPUT_MAN);
	  putchar(' ');
	}
	else if (strcmp(cname, name) && strcmp(cname, name + 1))
	  fputs("void ", stdout);

	fputs(name, stdout);

	for (arg = mxmlFindElement(function, function, "argument", NULL, NULL,
                        	   MXML_DESCEND_FIRST), prefix = '(';
	     arg;
	     arg = mxmlFindElement(arg, function, "argument", NULL, NULL,
                        	   MXML_NO_DESCEND), prefix = ',')
	{
	  type = mxmlFindElement(arg, arg, "type", NULL, NULL,
	                	 MXML_DESCEND_FIRST);

	  putchar(prefix);
	  if (prefix == ',')
	    putchar(' ');

	  if (type->child)
	  {
	    write_element(doc, type, OUTPUT_MAN);
	    putchar(' ');
	  }
	  fputs(mxmlElementGetAttr(arg, "name"), stdout);
          if ((defval = mxmlElementGetAttr(arg, "default")) != NULL)
	    printf(" %s", defval);
	}

	if (prefix == '(')
	  puts("(void);");
	else
	  puts(");");
      }

      puts("};\n.fi\n.PP");

      if (description)
	write_description(description, OUTPUT_MAN);
    }
  }

 /*
  * List of types...
  */

  if (find_public(doc, doc, "typedef"))
  {
    puts(".SH TYPES");

    for (scut = find_public(doc, doc, "typedef");
	 scut;
	 scut = find_public(scut, doc, "typedef"))
    {
      name        = mxmlElementGetAttr(scut, "name");
      description = mxmlFindElement(scut, scut, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf(".SS %s\n", name);

      fputs(".nf\n"
	    "typedef ", stdout);

      type = mxmlFindElement(scut, scut, "type", NULL, NULL,
                             MXML_DESCEND_FIRST);

      for (type = type->child; type; type = type->next)
        if (!strcmp(type->value.text.string, "("))
	  break;
	else
	{
	  if (type->value.text.whitespace)
	    putchar(' ');

          write_string(type->value.text.string, OUTPUT_MAN);
        }

      if (type)
      {
       /*
        * Output function type...
	*/

        printf(" (*%s", name);

	for (type = type->next->next; type; type = type->next)
	{
	  if (type->value.text.whitespace)
	    putchar(' ');

          write_string(type->value.text.string, OUTPUT_MAN);
        }

        puts(";");
      }
      else
	printf(" %s;\n", name);

      puts(".fi\n.PP");

      if (description)
	write_description(description, OUTPUT_MAN);
    }
  }

 /*
  * List of unions...
  */

  if (find_public(doc, doc, "union"))
  {
    puts(".SH UNIONS");

    for (scut = find_public(doc, doc, "union");
	 scut;
	 scut = find_public(scut, doc, "union"))
    {
      name        = mxmlElementGetAttr(scut, "name");
      description = mxmlFindElement(scut, scut, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf(".SS %s\n", name);

      printf(".nf\n"
	     "union %s\n{\n", name);
      for (arg = mxmlFindElement(scut, scut, "variable", NULL, NULL,
                        	 MXML_DESCEND_FIRST);
	   arg;
	   arg = mxmlFindElement(arg, scut, "variable", NULL, NULL,
                        	 MXML_NO_DESCEND))
      {
	printf("  ");
	write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                           NULL, MXML_DESCEND_FIRST),
                      OUTPUT_MAN);
	printf(" %s;\n", mxmlElementGetAttr(arg, "name"));
      }

      puts("};\n.fi\n.PP");

      if (description)
	write_description(description, OUTPUT_MAN);
    }
  }

 /*
  * Variables...
  */

  if (find_public(doc, doc, "variable"))
  {
    puts(".SH VARIABLES");

    for (arg = find_public(doc, doc, "variable");
	 arg;
	 arg = find_public(arg, doc, "variable"))
    {
      name        = mxmlElementGetAttr(arg, "name");
      description = mxmlFindElement(arg, arg, "description", NULL,
                                    NULL, MXML_DESCEND_FIRST);
      printf(".SS %s\n", name);

      puts(",nf");

      write_element(doc, mxmlFindElement(arg, arg, "type", NULL,
                                         NULL, MXML_DESCEND_FIRST),
                    OUTPUT_MAN);
      printf(" %s", mxmlElementGetAttr(arg, "name"));
      if ((defval = mxmlElementGetAttr(arg, "default")) != NULL)
	printf(" %s", defval);
      puts(";\n.fi\n.PP");

      if (description)
	write_description(description, OUTPUT_MAN);
    }
  }
}


/*
 * 'write_string()' - Write a string, quoting HTML special chars as needed...
 */

static void
write_string(const char *s,		/* I - String to write */
             int        mode)		/* I - Output mode */
{
  switch (mode)
  {
    case OUTPUT_HTML :
        while (*s)
        {
          if (*s == '&')
            fputs("&amp;", stdout);
          else if (*s == '<')
            fputs("&lt;", stdout);
          else if (*s == '>')
            fputs("&gt;", stdout);
          else if (*s == '\"')
            fputs("&quot;", stdout);
          else if (*s & 128)
          {
           /*
            * Convert UTF-8 to Unicode constant...
            */

            int	ch;			/* Unicode character */


            ch = *s & 255;

            if ((ch & 0xe0) == 0xc0)
            {
              ch = ((ch & 0x1f) << 6) | (s[1] & 0x3f);
	      s ++;
            }
            else if ((ch & 0xf0) == 0xe0)
            {
              ch = ((((ch * 0x0f) << 6) | (s[1] & 0x3f)) << 6) | (s[2] & 0x3f);
	      s += 2;
            }

            if (ch == 0xa0)
            {
             /*
              * Handle non-breaking space as-is...
	      */

              fputs("&nbsp;", stdout);
            }
            else
              printf("&#x%x;", ch);
          }
          else
            putchar(*s);

          s ++;
        }
        break;

    case OUTPUT_MAN :
        while (*s)
        {
          if (*s == '\\')
            putchar('\\');

          putchar(*s++);
        }
        break;
  }
}


/*
 * 'ws_cb()' - Whitespace callback for saving.
 */

static const char *			/* O - Whitespace string or NULL for none */
ws_cb(mxml_node_t *node,		/* I - Element node */
      int         where)		/* I - Where value */
{
  const char *name;			/* Name of element */
  int	depth;				/* Depth of node */
  static const char *spaces = "                                        ";
					/* Whitespace (40 spaces) for indent */


  name = node->value.element.name;

  switch (where)
  {
    case MXML_WS_BEFORE_CLOSE :
        if (strcmp(name, "argument") &&
	    strcmp(name, "class") &&
	    strcmp(name, "constant") &&
	    strcmp(name, "enumeration") &&
	    strcmp(name, "function") &&
	    strcmp(name, "mxmldoc") &&
	    strcmp(name, "namespace") &&
	    strcmp(name, "returnvalue") &&
	    strcmp(name, "struct") &&
	    strcmp(name, "typedef") &&
	    strcmp(name, "union") &&
	    strcmp(name, "variable"))
	  return (NULL);

	for (depth = -4; node; node = node->parent, depth += 2);
	if (depth > 40)
	  return (spaces);
	else if (depth < 2)
	  return (NULL);
	else
	  return (spaces + 40 - depth);

    case MXML_WS_AFTER_CLOSE :
	return ("\n");

    case MXML_WS_BEFORE_OPEN :
	for (depth = -4; node; node = node->parent, depth += 2);
	if (depth > 40)
	  return (spaces);
	else if (depth < 2)
	  return (NULL);
	else
	  return (spaces + 40 - depth);

    default :
    case MXML_WS_AFTER_OPEN :
        if (strcmp(name, "argument") &&
	    strcmp(name, "class") &&
	    strcmp(name, "constant") &&
	    strcmp(name, "enumeration") &&
	    strcmp(name, "function") &&
	    strcmp(name, "mxmldoc") &&
	    strcmp(name, "namespace") &&
	    strcmp(name, "returnvalue") &&
	    strcmp(name, "struct") &&
	    strcmp(name, "typedef") &&
	    strcmp(name, "union") &&
	    strcmp(name, "variable"))
	  return (NULL);
	else
          return ("\n");
  }
}


/*
 * End of "$Id$".
 */
