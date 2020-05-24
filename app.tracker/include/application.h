#ifndef APPLICATION_H
#define APPLICATION_H

class IApplication
{
public:
    virtual ~IApplication();
    
    virtual bool Setup() = 0;
    virtual void Render3d() = 0;
    virtual void Render2d() = 0;
    virtual void Cleanup() = 0;
};

#endif // APPLICATION_H
