
class Widget
{
    Widget() { }
    ~Widget() { }

    void OnCreate() { }
    void OnDestruct() { }
    void OnUpdate(float ts) { }
    void OnChildAdd(Widget@ widget) { }
    void OnChildRemove(Widget@ widget) { }

    protected UIElement@ Proxy
    {
        get
        {
            return m_Proxy;
        }
    }

    private UIElement@ m_Proxy;
}