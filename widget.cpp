#include "widget.h"
#include "ui_widget.h"
#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QtSvg/QSvgRenderer>
#include <DProgressBar>
#include <QFileDialog>
#include <QFont>
#include <QProcess>
#include <QFileInfo>
#include <QtConcurrent> //并发
#include <QDebug>
#include <QDir>
#include <QSettings> //读取desktop
#include <fstream>
#include <QDragEnterEvent>
#include <QRegExp>

DWIDGET_USE_NAMESPACE


using namespace std;
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    step=1;
    ui->setupUi(this);
    ui->label->setPixmap(QIcon::fromTheme("package-x-generic").pixmap(135,135));
    ui->label->setScaledContents(true);
    ui->progressBar->hide();
    ui->progressBar->setRange(0,100);
    QFont font_tip;
    ui->label_2->setStyleSheet("color:#8AA1B4");
    ui->label_2->setText("将AppImage软件包拖入以打开");
    ui->label_2->setWordWrap(true);
    setAcceptDrops(true);
    ui->widget_5->hide();
    QRegExp regx("[0-9]+[0-9a-zA-Z.-+~]{29}");  //使用正则表达式规范输入一定程度上防止用户非法输入
    QValidator *validator = new QRegExpValidator(regx, ui->lineEdit_2 );
    ui->lineEdit_2->setValidator( validator );
    ui->pushButton_2->hide();
    ui->checkBox->hide();
    qDebug()<<ui->checkBox->isChecked();
    ui->widget_4->hide();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::unpack()
{
    system("rm -r /tmp/squashfs-root");
    setAcceptDrops(false);
    QDir::setCurrent("/tmp/");
    QProcess process;
    process.start("cd /tmp");
    process.waitForFinished();
    system("chmod +x "+filePath.toUtf8());
    process.start(filePath.toUtf8() + " --appimage-extract");
    process.waitForFinished();
    //开始读取图标
    QProcess lspng;
    lspng.start("ls /tmp/squashfs-root/");
    lspng.waitForFinished();
    QString output=lspng.readAllStandardOutput();
    QStringList pngname = output.split("\n");
    for (int i = 0;i<pngname.size();i++) {
        if(pngname[i].right(4)==".png" || pngname[i].right(4)==".svg"){
            pngname[0]=pngname[i];
        }
    }
    qDebug()<<pngname[0];
    file_icon=pngname[0];
    if(pngname[0].right(3)=="png"){
        ui->label->setPixmap(QPixmap::fromImage(QImage("/tmp/squashfs-root/"+pngname[0])));
    }else {
        QString svgPath = "/tmp/squashfs-root/"+pngname[0].toUtf8();
        QSvgRenderer* svgRender = new QSvgRenderer();
        svgRender->load(svgPath);
        QPixmap* pixmap = new QPixmap(128,128);
        pixmap->fill(Qt::transparent);//设置背景透明
        QPainter p(pixmap);
        svgRender->render(&p);
        ui->label->setPixmap(*pixmap);
        ui->label->setAlignment(Qt::AlignHCenter);
    }
    //图标文件设置结束

    //开始读取软件名
    QProcess lsdesktop;
    lspng.start("ls /tmp/squashfs-root/");
    lspng.waitForFinished();
    QString output_desktop=lspng.readAllStandardOutput();
    QStringList desktop_name = output.split("\n");
    for (int i = 0;i<desktop_name.size();i++) {
        if(desktop_name[i].right(8)==".desktop"){
            desktop_name[0]=desktop_name[i];
        }
    }
    QSettings desktopname ("/tmp/squashfs-root/"+desktop_name[0],QSettings::IniFormat);
    ctrl_Version=desktopname.value(  "Desktop Entry/Version"  ).toString();
    ctrl_Description=desktopname.value(  "Desktop Entry/Comment"  ).toString();
    QFileInfo desktop_file("/tmp/squashfs-root/"+ desktop_name[0]) ;//获得desktop文件名作为包名，不包含后缀
    ctrl_Package=desktop_file.fileName().left(desktop_file.fileName().length()-8);

    //软件名读取结束
    ui->widget_5->show();
    ui->lineEdit->setText(ctrl_Package);

    ui->pushButton->setText("开始打包");
    ui->checkBox->show();

    ui->pushButton->setEnabled(true);
    step=2;
}

void Widget::pack()
{
    ui->progressBar->setValue(0);
    ui->progressBar->show();
    ui->widget_4->show();
    setAcceptDrops(false);
    system("rm -r /tmp/packdeb");
    ui->label_2->setText("正在创建目录结构");
    system("mkdir /tmp/packdeb");
    system("mkdir /tmp/packdeb/DEBIAN");
    system("touch /tmp/packdeb/DEBIAN/control");
    system("mkdir /tmp/packdeb/opt");
    system("mkdir /tmp/packdeb/opt/durapps");
    system("mkdir /tmp/packdeb/opt/durapps/a2d-packages");
    system("mkdir /tmp/packdeb/opt/durapps/a2d-packages/"+ctrl_Package.toUtf8());
    system("mkdir /tmp/packdeb/usr");
    system("mkdir /tmp/packdeb/usr/share");
    system("mkdir /tmp/packdeb/usr/share/applications");
    system("mkdir /tmp/packdeb/usr/share/icons/");
    ui->progressBar->setValue(10);
    ui->label_2->setText("正在复制文件");
    if(ui->checkBox->isChecked()){
        system("cp -r /tmp/squashfs-root/* /tmp/packdeb/opt/durapps/a2d-packages/"+ctrl_Package.toUtf8());
    }else {
        system("cp "+filePath.toUtf8()+" /tmp/packdeb/opt/durapps/a2d-packages/"+ctrl_Package.toUtf8());//部署可执行文件
    }

    ui->progressBar->setValue(20);
    //生成快捷方式
    QFile redesktop("/tmp/squashfs-root/"+ctrl_Package+".desktop");
    fstream newdesktop;
    newdesktop.open("/tmp/packdeb/usr/share/applications/"+ctrl_Package.toUtf8()+".desktop",ios::out);
    redesktop.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream txtInput(&redesktop);
    QString lineStr;
    while (!txtInput.atEnd())
    {
        lineStr = txtInput.readLine();  //读取数据
        if(lineStr.left(4)=="Exec"){
            if(ui->checkBox->isChecked()){
                lineStr="Exec=/opt/durapps/a2d-packages/"+ctrl_Package+"/AppRun";
            }else {
                lineStr="Exec=/opt/durapps/a2d-packages/"+ctrl_Package+"/"+imageName;//关联快捷
            }
        }
        if(lineStr.left(4)=="Icon"){
            lineStr="Icon=/opt/durapps/a2d-packages/"+ctrl_Package+"/"+file_icon;
        }
        newdesktop<<lineStr.toStdString()<<endl;

    }
    redesktop.close();
    newdesktop.close();
    ui->progressBar->setValue(30);
//    system("cp -r /tmp/squashfs-root/usr/share/icons/* /tmp/packdeb/usr/share/icons");//复制图标
    system("cp /tmp/squashfs-root/"+file_icon.toUtf8()+" /tmp/packdeb/opt/durapps/a2d-packages/"+ctrl_Package.toUtf8());
    qDebug()<<"cp /tmp/squashfs-root/"+file_icon.toUtf8()+" /tmp/packdeb/opt/durapps/a2d-packages/"+ctrl_Package.toUtf8();



    ui->progressBar->setValue(40);
    ui->label_2->setText("写入控制文件");
    fstream out_control;
    out_control.open("/tmp/packdeb/DEBIAN/control",ios::out);
    out_control<<"Package: "<<ctrl_Package.toStdString()<<endl;
    out_control<<"Version: "<<ctrl_Version.toStdString()<<endl;
    out_control<<"Architecture: "<<ctrl_Architecture.toStdString()<<endl;
    out_control<<"Description: "<<ctrl_Description.toStdString()<<endl;
    out_control<<"Depends: \n";
    out_control<<"Maintainer: community\n";
    out_control.close();
    ui->progressBar->setValue(80);
    update();
    ui->label_2->setText("正在打包,可能需要较长时间");

    if(ui->checkBox->isChecked()){
        system("dpkg -b /tmp/packdeb "+dirPath.toUtf8()+"/"+ctrl_Package.toUtf8()+"_"+ctrl_Version.toUtf8()+"_a2d.deb");
    }else {
        system("fakeroot dpkg -b /tmp/packdeb "+dirPath.toUtf8()+"/"+ctrl_Package.toUtf8()+"_"+ctrl_Version.toUtf8()+"_a2d.deb");
    }
    ui->progressBar->setValue(100);
    update();
    ui->progressBar->hide();
    ui->widget_4->hide();
    ui->label_2->setText("打包完成,请到原目录查看");
    ui->pushButton->setText("打开文件夹");
    ui->pushButton->setEnabled(true);
    ui->widget_5->hide();
    step=-2;
    fstream deepinInstaller;
    deepinInstaller.open("/usr/bin/deepin-deb-installer",ios::in);
    if(deepinInstaller){
        ui->pushButton_2->show();
    }
}

void Widget::dragEnterEvent(QDragEnterEvent * event)
{
        event->acceptProposedAction();
}

void Widget::dropEvent(QDropEvent *event)
{
    QString fileurl=event->mimeData()->urls().at(0).toString();
    qDebug()<<fileurl;
    if(fileurl.right(9)==".AppImage" || fileurl.right(9)==".appimage" || fileurl.right(9)==".APPIMAGE"){
        qDebug()<<fileurl.length();
        filePath=fileurl.right(fileurl.length()-7);
        QFileInfo fileInfo(filePath);
        filePath =fileInfo.path()+"/"+fileInfo.fileName();
        dirPath = fileInfo.absolutePath();
        imageName = fileInfo.fileName();
        ui->label_2->setText(filePath);
        ui->pushButton->setEnabled(false);
        ui->pushButton->setText("正在解析软件包");
        QtConcurrent::run([=](){
            unpack();
        });
    }
}

void Widget::on_pushButton_clicked()
{
    if(step==-1){
        QApplication::quit();
    }
    if(step==1){ //打开文件
        filePath = QFileDialog::getOpenFileName(nullptr,("打开文件"),QDir::homePath(),
                ("AppImage(*.appImage);;所有文件(*.*)"));
        if(filePath!=""){
            QFileInfo fileInfo(filePath);
            dirPath = fileInfo.absolutePath();
            imageName = fileInfo.fileName();
            ui->label_2->setText(filePath);
            ui->pushButton->setEnabled(false);
            ui->pushButton->setText("正在解析软件包");
            QtConcurrent::run([=](){  //并发防止无响应
                unpack();
            });
        }
    }
    if(step==2){ //开始打包
        ui->checkBox->hide();
        fstream fakerootIsHere;
        fakerootIsHere.open("/usr/bin/fakeroot",ios::in);
        if(fakerootIsHere){
            if(ui->lineEdit->text()!="" && ui->lineEdit_2->text()!="" && ui->lineEdit_2->text().length()>=2){
                ctrl_Package=ui->lineEdit->text();
                ctrl_Version=ui->lineEdit_2->text();
                ctrl_Architecture=ui->comboBox->currentText();
                ui->pushButton->setEnabled(false);
                ui->pushButton->setText("请稍等");
                ui->lineEdit->setEnabled(false);
                ui->lineEdit_2->setEnabled(false);
                ui->comboBox->setEnabled(false);
                QtConcurrent::run([=](){  //并发防止无响应
                    pack();
                });
            }else {
                system("notify-send \"请检查是否将包信息补充完整\"");
            }
        }else {
            system("notify-send \"你需要安装fakeroot来继续执行下面的操作\"");
        }
    }
    if(step==-2){ //打开文件夹
        system("dde-file-manager "+dirPath.toUtf8()+"&");
        QApplication::quit();
    }
}

void Widget::on_pushButton_2_clicked()
{
    system("deepin-deb-installer "+dirPath.toUtf8()+"/"+ctrl_Package.toUtf8()+"_a2d.deb&");
    QApplication::quit();
}

void Widget::on_checkBox_clicked(bool checked)
{
    qDebug()<<checked;
    if(checked){
        ui->label_2->setText("解包后打包会使软件打开速度变快，\n但是部分软件可能因此无法运行。");
    }else {
        ui->label_2->setText(filePath);
    }

}
