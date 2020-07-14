#include "mainwindow.h"
#include <DApplication>
#include <DWidgetUtil>  //Dtk::Widget::moveToCenter(&w); 要调用它，就得引用DWidgetUtil

DWIDGET_USE_NAMESPACE
int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();  //让bar处在标题栏中
    DApplication a(argc, argv);

     a.setAttribute(Qt::AA_UseHighDpiPixmaps);
     a.loadTranslator();
     a.setOrganizationName("DCS");
     a.setApplicationVersion(DApplication::buildVersion("0.7+spark"));
     a.setApplicationAcknowledgementPage("https://gitee.com/deepin-community-store/AppImageToDeb");
     a.setProductIcon(QIcon("://builder.png"));  //设置Logo
     a.setProductName("Appimage投稿工具");
     a.setApplicationName("A2D投稿"); //只有在这儿修改窗口标题才有效
     a.setApplicationDescription("社区商店Appimage转制deb包工具，真正实现了有Appimage就可以投稿");
    MainWindow w;
    w.show();

    //让打开时界面显示在正中
    Dtk::Widget::moveToCenter(&w);


    return a.exec();
}
