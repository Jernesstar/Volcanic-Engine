#include "User.asx"

auto element = Magma.CreateElement("Text", { { "label", "Piece of text" } });

UIComponent HomePage() {
    auto __root_0 = Magma.CreateRootElement();
    auto __title_0 = __root_0.CreateElement("Title", { { "text", "This is the homepage" }, { "class", className } });
    auto __header_0 = __root_0.CreateElement("Header", { }, username + "_user");
    auto __text_box = __root_0.CreateElement("TextBox", { }, "This is the grey text {{data}}");
    auto __root_1 = __root_0.CreateElement("Container", { { "style", "Horizontal" } });
    {
        __root_1.CreateElement("Button" { { "style", "HomeButton" }, { "icon", "HomeButton.svg" } });

        if(UserLoggedIn)
            __root_1.CreateElement("Button" { { "style", "AccountButton" }, { "icon", "AccountButton.svg" } });
        else
            __root_1.CreateElement("Button" { { "style", "LogInButton" }, { "icon", "LogInButton.svg" } });
    }
    auto __root_2 = __root_0.CreateElement("Container", { { "style", "Vertical" } });
    {
        auto projects = Projects.Top(5);
        for(uint i = 0; i < projects.Count(); i++) {
            auto p = projects[i];
            __root_2.CreateElement("Label", { { "text", p.Name } });
            __root_2.CreateElement("Icon", { { "path", "ProjectIcon.svg" } });
        }
    }

    return __root_0;
}