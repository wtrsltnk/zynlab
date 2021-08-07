#ifndef IAPPLICATION_H
#define IAPPLICATION_H

class IApplication
{
public:
    virtual ~IApplication();

    virtual bool Setup() = 0;
    virtual void Render2d() = 0;
    virtual void Cleanup() = 0;

    int Width() const;
    int Height() const;

    void PostRedraw();
};

#endif // IAPPLICATION_H
