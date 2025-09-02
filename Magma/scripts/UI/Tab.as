#include "Panel.as"

shared abstract class Tab
{
    string Name
    {
        get { return TabHandle.Name; }
        set { TabHandle.Name = value; }
    }

    void OnSelect() { }
    void OnDeselect() { }
    void OnClose() { }
    void OnUpdate(float ts) { }
    void OnRender() { }

    Panel_t@ GetPanel(string name) { return TabHandle.GetPanel(name); }

    private Tab_t@ TabHandle;
}