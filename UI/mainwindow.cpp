#include <QtWidgets/QProgressBar>
#include <QtWidgets/QHBoxLayout>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupList(ui->listWidget1_1,ui->listWidget1_2,ui->listWidget1_3,localFiles);

    //ls接口设计的不够简洁，造成过量的冗余
    SOCKET datasock=pasv(CommandSocket);
    vector<File> serverList=ls(CommandSocket,datasock);
    setupList(ui->listWidget2_1,ui->listWidget2_2,ui->listWidget2_3,serverList);

    //移除多余的自动tab标签生成页
    ui->tabWidget_2->removeTab(1);
    ui->tabWidget->removeTab(1);

    //增加四个主要工具按钮的icon路径
    QIcon upload("../UI/resoucre/icon/48/upload.png");
    ui->pushButton_upload->setIcon(upload);
    QIcon download("../UI/resoucre/icon/48/download.png");
    ui->pushButton_download->setIcon(download);
    QIcon stop("../UI/resoucre/icon/48/stop.png");
    ui->pushButton_stop->setIcon(stop);
    QIcon cancel("../UI/resoucre/icon/48/cancel.png");
    ui->pushButton_cancel->setIcon(cancel);
    // TODO 还应该对四个按钮的槽函数进行编写，在下面的函数里设置好了，但是目前是空的
    // TODO 注意的是对于stop按钮，点击后应变成continue按钮.continue还能切换回stop

    //      暂时不添加restart按钮

    // Important : 新增的自定义槽连接，用于同步三个列表。
    connect(ui->listWidget2_3->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(on_list2_3_scrollBar_value_changed(int)));
    connect(ui->listWidget1_3->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(on_list1_3_scrollBar_value_changed(int)));

    //避免重复多次定义同一个槽和连接，会爆炸。
    QItemDelegate* delegate=new QItemDelegate(ui->listWidget1_1);
    //只需要将commitData信号牵引出来就可以了
    connect(delegate,SIGNAL(commitData(QWidget*)),this,SLOT(on_local_list_edited(QWidget*)));
    ui->listWidget1_1->setItemDelegate(delegate);

    //远程端代理，处理编辑事件
    QItemDelegate* remoteDelegate=new QItemDelegate(ui->listWidget2_1);
    connect(remoteDelegate,SIGNAL(commitData(QWidget*)),this,SLOT(on_remote_list_edited(QWidget*)));
    ui->listWidget2_1->setItemDelegate(remoteDelegate);

    //本地右键菜单设置
    QAction *addDir=new QAction("新建文件夹",ui->listWidget1_1);
    QAction *delItem=new QAction("删除",ui->listWidget1_1);
    connect(addDir,SIGNAL(triggered()),this,SLOT(on_localMenu_addDir_triggered()));
    connect(delItem,SIGNAL(triggered()),this,SLOT(on_localMenu_delItem_triggered()));
    ui->listWidget1_1->addAction(addDir);
    ui->listWidget1_1->addAction(delItem);

    //远程右键菜单设置
    QAction *remoteAddDir=new QAction("新建文件夹",ui->listWidget2_1);
    QAction *remoteDelItem=new QAction("删除",ui->listWidget2_1);
    connect(remoteAddDir,SIGNAL(triggered()),this,SLOT(on_remoteMenu_addDir_triggered()));
    connect(remoteDelItem,SIGNAL(triggered()),this,SLOT(on_remoteMenu_delItem_triggered()));
    ui->listWidget2_1->addAction(remoteAddDir);
    ui->listWidget2_1->addAction(remoteDelItem);

}

//   两个辅助函数，应设计进类中
string getFileType(string name) {
    int i = name.find_last_of(".");
    return name.substr(i + 1);
}

//需检查是否生效
void setFileIcon(QListWidgetItem* item,string type)
{
    string iconPath="../UI/resoucre/icon/48/";
    iconPath.append(type);
    iconPath.append("-ext.png");
    QFile file(iconPath.data());
    if(file.exists())
    {
        //找到匹配项目
        QIcon icon(iconPath.data());
        item->setIcon(icon);
    }
    else{
        //未添加的匹配类型
        QIcon  icon("../UI/resoucre/icon/48/unknown-ext.png");
        item->setIcon(icon);
    }
}
//
MainWindow::~MainWindow()
{
    delete ui;
}

//设置文件列表
void MainWindow::setupList(QListWidget* w1,QListWidget *w2,QListWidget*w3,vector<File> lists)
{
    //清空旧有数据
    w1->clear();
    w2->clear();
    w3->clear();
    for(int i=0;i<lists.size();i++)
    {
        if(lists[i].type==1)
        {
             if(lists[i].name==".")
                continue;
              QListWidgetItem*i1=new QListWidgetItem(w1);
              QVariant path=QString(lists[i].path.data());
              i1->setData(Qt::UserRole,path);
              QVariant isNew=0;
              i1->setData(Qt::UserRole+1,isNew);
              QVariant isDir=1;
              i1->setData(Qt::UserRole+2,isDir);
              QVariant size=qlonglong(lists[i].size);
              i1->setData(Qt::UserRole+3, size);
              QListWidgetItem*i2=new QListWidgetItem(w2);
              QListWidgetItem*i3=new QListWidgetItem(w3);
              //去掉当前目录的项
              i1->setText(lists[i].name.data());
              //icon是否生效需要进行检验
              QIcon  icon("../UI/resoucre/icon/48/dir.png");
              i1->setIcon(icon);
              i2->setText("");
              i3->setText("系统文件夹");

        } else{
            QListWidgetItem*i1=new QListWidgetItem(w1);
            QVariant path=QString(lists[i].path.data());
            i1->setData(Qt::UserRole,path);
            QVariant isNew=0;
            i1->setData(Qt::UserRole+1,isNew);
            QVariant isDir=0;
            i1->setData(Qt::UserRole+2,isDir);
            QVariant fsize=qlonglong(lists[i].size);
            i1->setData(Qt::UserRole+3, fsize);
            QListWidgetItem*i2=new QListWidgetItem(w2);
            QListWidgetItem*i3=new QListWidgetItem(w3);
            i1->setText(lists[i].name.data());
            string type=getFileType(lists[i].name);
            setFileIcon(i1,type);
            i3->setText(type.data());
            string size="";
            char* m=(char*)malloc(400);
            memset(m,0,400);
            sprintf(m,"%ld",lists[i].size);
            size.append(m);
            size.append("B");
            i2->setText(size.data());
            free(m);

        }
    }


}
void MainWindow::on_pushButton_upload_clicked() //点击上传
{
    QList<QListWidgetItem*> files= this->ui->listWidget1_1->selectedItems();
    vector<string> paths;
    vector<QListWidgetItem*> uploadItems;   //记录上传产生的目录item
    for(int i=0; i<files.size(); i++){
        string filePath=files.at(i)->data(Qt::UserRole).toString().toStdString();
        paths.push_back(filePath);  //得到string类型的vector，存储所有选中需要上传的文件或目录
        QListWidgetItem* i_name=new QListWidgetItem(ui->listWidget_name);   //一项文件的名字
        i_name->setText(files.at(i)->text());
        QListWidgetItem* i_status=new QListWidgetItem(ui->listWidget_status);   //文件状态（上传中、暂停）
        i_status->setText("上传中");

        QListWidgetItem* i_progress=new QListWidgetItem(ui->listWidget_progress);   //文件上传进度
        QWidget* w=new QWidget(ui->listWidget_progress);
        QHBoxLayout* layout=new QHBoxLayout(w);
        QProgressBar* progressBar=new QProgressBar(w);  //item内插入进度条
        progressBar->setGeometry(QRect(470, 31+161*i, 171, 31));
        progressBar->setValue(0);
        QPushButton* pushButton_pause=new QPushButton(w);   //item插入暂停/继续按钮
        pushButton_pause->setGeometry(QRect(670, 31+161*i, 31, 31));
        QIcon pause("../UI/resoucre/icon/48/stop.png");
        pushButton_pause->setIcon(pause);
        QPushButton* pushButton_terminate=new QPushButton(w);   //item插入终止按钮
        pushButton_terminate->setGeometry(QRect(720, 31+161*i, 31, 31));
        QIcon terminate("../UI/resoucre/icon/48/cancel.png");
        pushButton_terminate->setIcon(terminate);
        layout->addWidget(progressBar);
        layout->addWidget(pushButton_pause);
        layout->addWidget(pushButton_terminate);
        w->setLayout(layout);
        w->show();
        ui->listWidget_progress->setItemWidget(i_progress, w);
        ui->listWidget_progress->show();

        QListWidgetItem* i_size=new QListWidgetItem(ui->listWidget_size);   //文件大小
        i_size->setText(files[i]->data(Qt::UserRole+3).toString()+"B");
    }
    uploadThread* thread=new uploadThread(CommandSocket, paths);  //创建一个线程，用于完成后台的上传任务，防止页面卡死
    thread->start();
}

void MainWindow::on_pushButton_download_clicked()
{

}

void MainWindow::on_pushButton_stop_clicked()
{

}

void MainWindow::on_pushButton_cancel_clicked()
{

}

void MainWindow::QFileInfoListToVector(QFileInfoList *qlist, vector<File> *list) {

    list->clear();    //清空原有的list
    for(int i=0;i<qlist->size();i++)
    {
        File file;
        if(qlist->operator[](i).isFile())
        {
            file.size=qlist->operator[](i).size();
            file.name=qlist->operator[](i).fileName().toStdString();
            file.path=qlist->operator[](i).absoluteFilePath().toStdString();
            file.type=2; //普通文件
        }
        else{
            file.path=qlist->operator[](i).absoluteFilePath().toStdString();
            file.name=qlist->operator[](i).fileName().toStdString();
            file.type=1; //目录文件
        }
        list->push_back(file);
    }
}
void MainWindow::on_progressBar_valueChanged(int value)
{

}

void MainWindow::on_pushButton_pause_clicked()
{

}

void MainWindow::on_pushButton_terminate_clicked()
{

}

void MainWindow::on_listWidget_progress_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    cout<<"啊啊啊啊啊食屎啦！"<<ui->listWidget_progress->currentIndex().row()<<endl;
}
