<?php
//
// "$Id$"
//
// Software download page.
//

//
// Include necessary headers...
//

include_once "phplib/html.php";


// List of download servers...
$sitelist = array(
  "ftp://ftp.easysw.com/pub" => "California, USA via FTP",
  "http://ftp.easysw.com/pub" => "California, USA via HTTP",
  "http://www.nu6.org/_/mirror/ftp.easysw.com/pub" => "Gustavsberg, Sweden via HTTP",
  "http://ftp.funet.fi/pub/mirrors/ftp.easysw.com/pub" => "Espoo, Finland via HTTP",
  "ftp://ftp.funet.fi/pub/mirrors/ftp.easysw.com/pub" => "Espoo, Finland via FTP",
);


// Get the list of software files...
$fp    = fopen("data/software.md5", "r");
$files = array();

while ($line = fgets($fp, 255))
  $files[sizeof($files)] = trim($line);

fclose($fp);

// Get form data, if any...
if (array_key_exists("FILE", $_GET))
  $file = $_GET["FILE"];
else
  $file = "";

if (array_key_exists("SITE", $_GET))
{
  $site = $_GET["SITE"];
  setcookie("SITE", $site, time() + 90 * 86400, "/");
}
else if (array_key_exists("SITE", $_COOKIE) &&
         array_key_exists($_COOKIE["SITE"], $sitelist))
  $site = $_COOKIE["SITE"];
else
  $site = "";

if (array_key_exists("VERSION", $_GET))
  $version = $_GET["VERSION"];
else
{
  $data    = explode(" ", $files[0]);
  $version = $data[1];
}

// Show the standard header...
if ($site != "" && $file != "")
  html_header("Download", "$site/$file");
else
  html_header("Download");

html_start_links(1);

$curversion = "";
for ($i = 0; $i < sizeof($files); $i ++)
{
  // Grab the data for the current file...
  $data     = explode(" ", $files[$i]);
  $fversion = $data[1];

  if ($fversion != $curversion)
  {
    $curversion = $fversion;
    html_link("v$fversion", "$PHP_SELF?VERSION=$fversion");
  }
}

html_link("Subversion", "$PHP_SELF#SVN");

html_end_links();

// Show files or sites...
if ($file != "")
{
  print("<h1>Download</h1>\n");

  if ($site != "")
    print("<p>Your download should begin shortly. If not, please "
         ."<a href='$site/$file'>click here</a> to download the file "
	 ."from the current mirror.</p>\n");
  else
    print("<p>Please select a mirror site below to begin the download.</p>\n");

  print("<form action='$PHP_SELF' method='GET' name='download'>\n"
       ."<input type='hidden' name='FILE' value='"
       . htmlspecialchars($file, ENT_QUOTES) . "'>\n"
       ."<input type='hidden' name='VERSION' value='"
       . htmlspecialchars($version, ENT_QUOTES) . "'>\n");

  if ($site == "")
    print("<input type='radio' name='SITE' value='' checked>None<br>\n");

  reset($sitelist);
  while (list($key, $val) = each($sitelist))
  {
    print("<input type='radio' name='SITE' value='$key' "
         ."onClick='document.download.submit();'");
    if ($site == $key)
      print("  checked");
    print(">$val<br>\n");
  }

  if ($site != "")
    print("<p><input type='submit' value='Change Mirror Site'>\n");
  else
    print("<p><input type='submit' value='Select Mirror Site'>\n");

  print("</form>\n");
}
else
{
  // Show files...
  print("<h1>Releases</h1>\n");

  html_start_table(array("Version", "Filename", "Size", "MD5 Sum"));

  $curversion = "";

  for ($i = 0; $i < sizeof($files); $i ++)
  {
    // Grab the data for the current file...
    $data     = explode(" ", $files[$i]);
    $md5      = $data[0];
    $fversion = $data[1];
    $filename = $data[2];
    $basename = basename($filename);

    if ($fversion == $version)
    {
      $cs = "<th>";
      $ce = "</th>";
    }
    else
    {
      $cs = "<td align='center'>";
      $ce = "</td>";
    }

    if ($fversion != $curversion)
    {
      if ($curversion != "")
      {
        html_start_row("header");
	print("<th colspan='4'></th>");
	html_end_row();
      }

      $curversion = $fversion;
      html_start_row();
      print("$cs<a name='$fversion'>$fversion</a>$ce");
    }
    else
    {
      html_start_row();
      print("$cs$ce");
    }

    if (file_exists("/home/ftp.easysw.com/pub/$filename"))
      $kbytes = (int)((filesize("/home/ftp.easysw.com/pub/$filename") + 1023) / 1024);
    else
      $kbytes = "???";

    print("$cs<a href='$PHP_SELF?VERSION=$version&amp;FILE=$filename'>"
         ."<tt>$basename</tt></a>$ce"
	 ."$cs${kbytes}k$ce"
	 ."$cs<tt>$md5</tt>$ce");

    html_end_row();
  }

  html_end_table();

  print("<h1><a name='SVN'>Subversion Access</a></h1>\n"
       ."<p>The $PROJECT_NAME software is available via Subversion "
       ."using the following URL:</p>\n"
       ."<pre>\n"
       ."    <a href='http://svn.easysw.com/public/$PROJECT_MODULE/'>"
       ."http://svn.easysw.com/public/$PROJECT_MODULE/</a>\n"
       ."</pre>\n"
       ."<p>The following command can be used to checkout the current "
       ."$PROJECT_NAME source from Subversion:</p>\n"
       ."<pre>\n"
       ."    <kbd>svn co http://svn.easysw.com/public/$PROJECT_MODULE/trunk/ $PROJECT_MODULE</kbd>\n"
       ."</pre>\n");
}

// Show the standard footer...
html_footer();

//
// End of "$Id$".
//
?>
