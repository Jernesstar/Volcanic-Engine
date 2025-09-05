
shared abstract class Panel
{
    bool Open = false;

    Panel()
    {
    }

    void OnOpen() { }
    void OnClose() { }
    void OnUpdate(float ts) { }
    void OnRender() { }

    // Panel@ GetPanel(string name) { return Panel.GetPanel(name); }

    private Panel_t@ PanelHandle;
};
