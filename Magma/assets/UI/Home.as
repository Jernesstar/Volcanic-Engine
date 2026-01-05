
class Home : Widget
{
    void OnCreate()
    {
        Widget@ b1 =
            WidgetManager.Create("Button", { "class", "Control" }, "close-button");
        b1.SetEvent(
            UIEvent::OnClick, function() { App.Close(); })
        Widget@ c = WidgetManager.Create("Container", { "TopBar" },
        [
            
        ]);
    }

    void OnUpdate(float ts)
    {

    }

    void OnRender()
    {
        
    }
}