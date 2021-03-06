 #include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdlib.h>
#include <QColor>
#include <QGuiApplication>
#include <QDebug>
#include <QVariant>
#include <QMessageBox>
#include <QFileDialog>
#include <QStringList>
#include <QTextStream>
#include <QTime>
#include <QCoreApplication>
#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QDateTime>
#include <QApplication>
#include <QDesktopWidget>
#include <QInputDialog>
#include <QTextBlock>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/machine_tool_ico.ico"));



    //Initialize the timer
    timer=new QTimer(this);
    timer->start(30);

    timer1=new QTimer(this);

    //Initialize the widgets of UI
    mainButton[0]=ui->pB_data;
    mainButton[1]=ui->pB_manul;
    mainButton[2]=ui->pB_program;
    mainButton[3]=ui->pB_manufacture;
    mainButton[4]=ui->pB_view;
    mainButton[5]=ui->pB_setting;
    subButton[0]=ui->pB2_0;
    subButton[1]=ui->pB2_1;
    subButton[2]=ui->pB2_2;
    subButton[3]=ui->pB2_3;
    subButton[4]=ui->pB2_4;
    sub2Button[0]=ui->pB3_0;
    sub2Button[1]=ui->pB3_1;
    sub2Button[2]=ui->pB3_2;
    sub2Button[3]=ui->pB3_3;
    sub2Button[4]=ui->pB3_4;
    sub2Button[5]=ui->pB3_5;
    sub2Button[6]=ui->pB3_6;
    sub2Button[7]=ui->pB3_7;

    ButtonGroup_main=new QButtonGroup();
    ButtonGroup_main->setExclusive(true);
    ButtonGroup_main->addButton(mainButton[0],0);
    ButtonGroup_main->addButton(mainButton[1],1);
    ButtonGroup_main->addButton(mainButton[2],2);
    ButtonGroup_main->addButton(mainButton[3],3);
    ButtonGroup_main->addButton(mainButton[4],4);
    ButtonGroup_main->addButton(mainButton[5],5);
    ButtonGroup_sub=new QButtonGroup();
    ButtonGroup_sub->setExclusive(true);
    ButtonGroup_sub->addButton(subButton[0],0);
    ButtonGroup_sub->addButton(subButton[1],1);
    ButtonGroup_sub->addButton(subButton[2],2);
    ButtonGroup_sub->addButton(subButton[3],3);
    ButtonGroup_sub->addButton(subButton[4],4);
    ButtonGroup_sub2=new QButtonGroup();
    ButtonGroup_sub2->setExclusive(true);
    ButtonGroup_sub2->addButton(sub2Button[0],0);
    ButtonGroup_sub2->addButton(sub2Button[1],1);
    ButtonGroup_sub2->addButton(sub2Button[2],2);
    ButtonGroup_sub2->addButton(sub2Button[3],3);
    ButtonGroup_sub2->addButton(sub2Button[4],4);
    ButtonGroup_sub2->addButton(sub2Button[5],5);
    ButtonGroup_sub2->addButton(sub2Button[6],6);
    ButtonGroup_sub2->addButton(sub2Button[7],7);
    for(int i=0;i<5;i++)
    {
        mainButton[i]->setAutoFillBackground(true);
    }
    for(int i=0;i<4;i++)
    {
        subButton[i]->setAutoFillBackground(true);
    }
    for(int i=0;i<7;i++)
    {
        sub2Button[i]->setAutoFillBackground(true);
    }

    this->setAutoFillBackground(true);

    Palette_Unpressed=QGuiApplication::palette();
    Palette_Pressed=QGuiApplication::palette();
    Palette_Unconnected=QGuiApplication::palette();
    Palette_Connected=QGuiApplication::palette();
    Palette_Pressed.setBrush(QPalette::Button,QColor(Qt::black));
    Palette_Pressed.setBrush(QPalette::ButtonText,QColor(Qt::red));
    Palette_Unconnected.setBrush(QPalette::Text,QColor(Qt::white));
    Palette_Unconnected.setBrush(QPalette::Background,QColor(Qt::red));
    Palette_Connected.setBrush(QPalette::Text,QColor(Qt::white));
    Palette_Connected.setBrush(QPalette::Background,QColor(Qt::green));

    current_ButtonID_main=-1;
    current_ButtonID_sub=-1;
    current_ButtonID_sub2=-1;
    mainStack=ui->Stacked_Pages_Main;
    editStack=ui->Stacked_Pages_Sub;
    Label_Connection_Status=ui->Label_Connection_Status;
    Label_Connection_Status->setAutoFillBackground(true);
    Label_Connection_Status->setPalette(Palette_Unconnected);

    dir_of_txt=QDir::root();
    Connection_Status_of_Trio=false;

    //Initialize things about Multi-Threads
    THREAD_CCD=new QThread();
    ccd=new thread_CCD();
    ccd->moveToThread(THREAD_CCD);

    THREAD_TRIO=new QThread();
    trio_MC664=new thread_Trio();
    trio_MC664->moveToThread(THREAD_TRIO);

    THREAD_ASSIST=new QThread();
    Assist=new thread_assist();
    Assist->moveToThread(THREAD_ASSIST);

    THREAD_OPENCV=new QThread();
    Opencv=new thread_opencv();
    Opencv->moveToThread(THREAD_OPENCV);

    GL_CCD=ui->openGLWidget_CCDImage;
    tool_Aiming=false;
    GL_zoom=ui->openGLWidget_zoomImage;
    zoomFactor_Slider=ui->horizontalSlider_Zoom_Factor;
    zoomFactor_Slider->setValue(50);
    zoomFactor_Slider->setMaximum(99);
    zoomFactor_Slider->setMinimum(50);
    ui->label_Zoom_Factor->setText(QString::number(zoomFactor_Slider->value()));

    //Default UI setting
    ui->Stacked_Pages_Main->setCurrentIndex(0);
    ui->Stacked_Pages_Sub->setCurrentIndex(0);
    Label_StatusBar.setText(QString::fromLocal8Bit("欢迎使用曲线磨削数控系统"));
    ui->statusBar->addWidget(&Label_StatusBar);

    //cutter setting
    cutter_para=ui->tableWidget_28;
    cutter_para->setColumnCount(2);
    cutter_para->setRowCount(4);
    QStringList header;
    header<<"No"<<QString::fromLocal8Bit("刀具半径");
    cutter_para->setHorizontalHeaderLabels(header);
    cutter_para->verticalHeader()->setVisible(false);
    cutter_para->setItem(0,0,new QTableWidgetItem("1"));
    cutter_para->setItem(1,0,new QTableWidgetItem("2"));
    cutter_para->setItem(2,0,new QTableWidgetItem("3"));
    cutter_para->setItem(3,0,new QTableWidgetItem("4"));
    cutter_para->setItem(0,1,new QTableWidgetItem("0.25"));
    cutter_para->setItem(1,1,new QTableWidgetItem("0.5"));
    cutter_para->setItem(2,1,new QTableWidgetItem("0.75"));
    cutter_para->setItem(3,1,new QTableWidgetItem("1"));
    QFont font = cutter_para->horizontalHeader()->font();
    font.setBold(true);
    cutter_para->horizontalHeader()->setFont(font);
    cutter_para->horizontalHeader()->resizeSection(0,100); //设置表头第一列的宽度为100
    cutter_para->horizontalHeader()->setStyleSheet("QHeaderView::section{background:white;}"); //设置表头背景色

    //Connect the signals and slots
    connect(timer,SIGNAL(timeout()),this,SLOT(current_time_text_set()));
    connect(timer,SIGNAL(timeout()),this,SLOT(request_Trio_parameters_request()));

    connect(ButtonGroup_main,SIGNAL(buttonClicked(int)),this,SLOT(pressed_mainButtonGroup(int)));
    connect(ButtonGroup_sub,SIGNAL(buttonClicked(int)),this,SLOT(pressed_subButtonGroup(int)));
    connect(ButtonGroup_sub2,SIGNAL(buttonClicked(int)),this,SLOT(pressed_sub2ButtonGroup(int)));

    connect(ui->pB_New_Txt,SIGNAL(clicked()),this,SLOT(txtfile_new_built()));
    connect(ui->pB_ReadIn_Txt,SIGNAL(clicked()),this,SLOT(txtfile_readin()));
    connect(ui->pB_Save_Txt,SIGNAL(clicked()),this,SLOT(txtfile_save()));
    connect(ui->pB_Undo_Txt,SIGNAL(clicked()),this,SLOT(txtfile_undo()));
    connect(ui->pB_GrammarCheck_Txt,SIGNAL(clicked()),this,SLOT(txtfile_grammar_check()));
    connect(ui->pB_Send_Txt,SIGNAL(clicked()),this,SLOT(txtfile_send_to_trio()));
    //connect(this,SIGNAL(cB_Txt_Changed(QString)),this,SLOT(cB_Txt_Dir_Content(QString)));
    connect(this,SIGNAL(label_Txt_Changed(QString)),this,SLOT(label_Txt_Content(QString)));
    //connect(ui->cB_Txt,SIGNAL(activated(QString)),this,SLOT(cB_current_index_changed(QString)));

    connect(this,SIGNAL(errors_in_runtime(int)),this,SLOT(errors_handled(int)));
    connect(ui->pB_Connection_of_Trio,SIGNAL(clicked()),this,SLOT(pB_Connection()));


    connect(THREAD_CCD,SIGNAL(finished()),ccd,SLOT(deleteLater()));
    connect(THREAD_TRIO,SIGNAL(finished()),trio_MC664,SLOT(deleteLater()));
    connect(THREAD_ASSIST,SIGNAL(finished()),Assist,SLOT(deleteLater()));
    connect(THREAD_OPENCV,SIGNAL(finished()),Opencv,SLOT(deleteLater()));

    connect(this,SIGNAL(initialize_ccd()),ccd,SLOT(initialize()));
    connect(this,SIGNAL(capture_picture()),ccd,SLOT(capture_image()));
    //connect(ccd,SIGNAL(image_captured(uchar*)),this,SLOT(receive_captured_image(uchar*)));
    connect(ccd,SIGNAL(image_captured(uchar*)),GL_CCD,SLOT(receive_Zoom_Image(uchar*)));
    connect(this,SIGNAL(return_OpenGLWidget_Zoom_Coordinate(int,int)),GL_zoom,SLOT(receive_OpenGLWidget_Zoom_Current_Coordinate(int,int)));
    //connect(GL_zoom,SIGNAL(return_Origin_Coordinate(int,int)),this,SLOT(receive_Origin_Coordinate(int,int)));
    connect(zoomFactor_Slider,SIGNAL(valueChanged(int)),this,SLOT(receive_Slider_Value(int)));
    connect(this,SIGNAL(stop_ccd()),ccd,SLOT(set_output_img()));

    connect(trio_MC664,SIGNAL(return_error_of_trio(int,QString,QString,QString)),
            this,SLOT(errors_of_trio_handled(int,QString,QString,QString)));
    connect(this,SIGNAL(call_Trio_connect(bool*)),trio_MC664,SLOT(connect_Trio(bool*)));
    connect(this,SIGNAL(call_Trio_close()),trio_MC664,SLOT(close_Trio()));
    connect(this,SIGNAL(call_Trio_run_program(bool*,QString)),trio_MC664,SLOT(run_program_of_Trio(bool*,QString)));
    connect(this,SIGNAL(call_Trio_send_txt(bool*,QString,QString)),
            trio_MC664,SLOT(send_txt_to_Trio(bool*,QString,QString)));
    connect(trio_MC664,SIGNAL(return_paras(bool*,QVariant)),this,SLOT(receive_Trio_axis_paras(bool*,QVariant)));
    connect(this,SIGNAL(call_Trio_run_MANUAL_MODE(bool*)),trio_MC664,SLOT(run_program_MANUAL_MODE(bool*)));
    connect(this,SIGNAL(call_Trio_return_axis_paras(bool*)),trio_MC664,SLOT(grab_axis_paras(bool*)));
    connect(this,SIGNAL(call_Trio_write_VR(int,double)),trio_MC664,SLOT(write_VR(int,double)));
    connect(this,SIGNAL(call_Trio_write_TABLE(int,double)),trio_MC664,SLOT(write_TABLE(int,double)));
    //connect(timer,SIGNAL(timeout()),this,SLOT(request_Trio_parameters_request()));
    connect(trio_MC664,SIGNAL(return_txt_transfer_situation(bool)),this,SLOT(receive_Trio_txt_transfer_situation(bool)));

    //realtime paint
    realtime_widget=ui->widget_realtime;
    connect(this,SIGNAL(return_QPointF(QPointF)),realtime_widget,SLOT(paint_track(QPointF)));

    //connect(Assist,SIGNAL(return_current_time_str(QString*)),this,SLOT(receive_current_time(QString*)));
    //connect(this,SIGNAL(call_start_time_loop()),Assist,SLOT(send_current_Time()));
    connect(this,SIGNAL(call_stop_time_loop()),Assist,SLOT(receive_time_loop_stop_flag()));

    //opencv相关
    connect(this,SIGNAL(start_opencv()),this,SLOT(opencv_parameters_trans()));
    connect(this,SIGNAL(start_grabuv()),this,SLOT(return_uv()));
    connect(this,SIGNAL(Imgdata_trans(Mat)),Opencv,SLOT(image_process(Mat)),Qt::DirectConnection);
    connect(this,SIGNAL(Posdata_trans(float,float,float)),Opencv,SLOT(PosData_re(float,float,float)));
    connect(Opencv,SIGNAL(set_com(bool,float,float)),this,SLOT(compensation_value_set(bool,float,float)));

    connect(GL_CCD,SIGNAL(transmit_Current_Image(QImage&)),GL_zoom,SLOT(receive_Current_Image(QImage&)));

    THREAD_CCD->start();
    THREAD_TRIO->start();
    THREAD_ASSIST->start();
    THREAD_OPENCV->start();

    //Start assistant thread,sending the current time
    emit call_start_time_loop();

    photo_ok=false;
    grab_pic=false;

    //showmistake&track
    MistakeUi = new ShowMistake;
    connect(MistakeUi, SIGNAL(sendLine(int)), this, SLOT(lineChoose(int)));
    connect(MistakeUi, SIGNAL(sendOrder(QString)), this, SLOT(OrderRevise(QString)));

    TrackUi = new track;

    //opencv_processing();

    //layout setting
    int screenHeight = QApplication::desktop()->availableGeometry().height();
    //int screenWidth = QApplication::desktop()->availableGeometry().width();
    //int screenHeight = QApplication::desktop()->height();

    //this->setMaximumHeight(screenHeight*23/25);
    //this->setMaximumWidth(screenWidth);

    ui->centralWidget->setMaximumHeight(screenHeight*23/25);



    ui->Stacked_Pages_Main->setMaximumHeight(screenHeight*4/5);

    ui->Stacked_Pages_Sub->setMaximumHeight(screenHeight*7/25);

    ui->Label_Img->setMaximumWidth(screenHeight*13/25*408/343);
    ui->Label_Img->setMaximumHeight(screenHeight*13/25);

    ui->tabWidget_4->setMaximumHeight(screenHeight*13/25);

    //ui->label_4->setMaximumWidth(screenHeight*4/5*408/343);
    //ui->label_4->setMaximumHeight(screenHeight*4/5);

    //ui->menuBar->setMaximumHeight(screenHeight*1/50);
    //ui->mainToolBar->setMaximumHeight(screenHeight*1/50);
    //ui->statusBar->setMaximumHeight(screenHeight*1/50);

    //simulate_path
    //sp = new simulate_path();
    currentLine=-1;


}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(ui->rB_CCD->isChecked())

    {
        //QMessageBox::warning(this,"Wrong","Please click");
        //ccd->set_output_img();
        ui->rB_CCD->setChecked(false);
        //     event->ignore();
    }

    Sleep(100);
    disconnect(this,SIGNAL(call_Trio_return_axis_paras(bool*)),trio_MC664,SLOT(grab_axis_paras(bool*)));
    if(timer->isActive())
    {
        timer->stop();
        disconnect(timer,SIGNAL(timeout()),this,SLOT(current_time_text_set()));
        disconnect(timer,SIGNAL(timeout()),this,SLOT(request_Trio_parameters_request()));
    }
    Sleep(1000);
    //    ccd->set_output_img();
    //   if (ccd->check_ccd_status())
    {

    }

    Assist->receive_time_loop_stop_flag();
    trio_MC664->close_Trio_Grabbing();

}

MainWindow::~MainWindow()
{
    //  emit stop_ccd();

    delete timer;

    for(int i=0;i<=5;i++)
    {
        delete mainButton[i];
    }
    for(int i=0;i<=4;i++)
    {
        delete subButton[i];
    }
    for(int i=0;i<=7;i++)
    {
        delete sub2Button[i];
    }

    delete ccd;
    THREAD_CCD->quit();
    THREAD_CCD->wait();
    delete THREAD_CCD;

    delete Opencv;
    THREAD_OPENCV->quit();
    THREAD_OPENCV->wait();
    delete THREAD_OPENCV;

    emit call_Trio_close();
    qSleep(1000);
    trio_MC664->disconnect();
    qSleep(1000);
    delete trio_MC664;
    THREAD_TRIO->quit();
    THREAD_TRIO->wait();
    delete THREAD_TRIO;

    emit call_stop_time_loop();
    delete Assist;
    THREAD_ASSIST->quit();
    THREAD_ASSIST->wait();
    delete THREAD_ASSIST;

    delete realtime_widget;

    delete GL_CCD;
    delete GL_zoom;

    delete ui;
}



void MainWindow::qSleep(int ms)
{
    common::qSleep(ms);
}

//*******************About Buttons*******************//
void MainWindow::pressed_mainButtonGroup(int i)
{
    bool ok(false);

    for(int j=0;j<=5;j++)
    {
        button_unpressed(mainButton[j]);
    }
    button_pressed(mainButton[i]);

    mainStack->setCurrentIndex(0);
    clear_button_text(ALL_SUB);
    switch (i) {
    case 0:
        subButton[0]->setText(QString::fromLocal8Bit("表格"));
        break;
    case 1:
        subButton[0]->setText(QString::fromLocal8Bit("准备"));
        subButton[1]->setText(QString::fromLocal8Bit("MDI"));
        subButton[2]->setText(QString::fromLocal8Bit("示教"));
        emit call_Trio_run_MANUAL_MODE(&ok);
        break;
    case 2:
        subButton[0]->setText(QString::fromLocal8Bit("文件"));
        subButton[1]->setText(QString::fromLocal8Bit("编辑"));
        subButton[2]->setText(QString::fromLocal8Bit("模拟"));
        subButton[3]->setText(QString::fromLocal8Bit("跳转"));
        subButton[4]->setText(QString::fromLocal8Bit("显示轨迹"));
        mainStack->setCurrentIndex(1);
        break;
    case 3:
        subButton[0]->setText(QString::fromLocal8Bit("文件"));
        subButton[1]->setText(QString::fromLocal8Bit("执行"));
        subButton[2]->setText(QString::fromLocal8Bit("  "));
        break;
    case 4:
        subButton[0]->setText(QString::fromLocal8Bit("视屏"));
        subButton[1]->setText(QString::fromLocal8Bit("状态"));
        subButton[2]->setText(QString::fromLocal8Bit("表格"));
        subButton[3]->setText(QString::fromLocal8Bit("  "));
        break;
    case 5:
        subButton[0]->setText(QString::fromLocal8Bit("参数设定"));
        subButton[1]->setText(QString::fromLocal8Bit("软盘处理"));
        subButton[2]->setText(QString::fromLocal8Bit("诊断"));
        break;
    default:
        clear_button_text(ALL_SUB);
        break;
    }
    current_ButtonID_main=i;//Remember the necessary main button Group ID
}

void MainWindow::pressed_subButtonGroup(int i)
{
    for(int j=0;j<=4;j++)
    {
        button_unpressed(subButton[j]);
    }
    button_pressed(subButton[i]);

    clear_button_text(SUB2);

    switch(current_ButtonID_main)
    {
    case 0:
        switch(i)
        {
        case 0:
            sub2Button[0]->setText(QString::fromLocal8Bit("零点偏置"));
            sub2Button[1]->setText(QString::fromLocal8Bit("刀具偏置"));
            sub2Button[2]->setText(QString::fromLocal8Bit("修正补偿"));
            sub2Button[3]->setText(QString::fromLocal8Bit("滑板往复"));
            sub2Button[4]->setText(QString::fromLocal8Bit("误差补偿"));
            break;
        default:

            break;
        }
        break;
    case 1:
        switch(i)
        {
        case 0:
            sub2Button[0]->setText(QString::fromLocal8Bit("回零点"));
            sub2Button[1]->setText(QString::fromLocal8Bit("坐标预设"));
            sub2Button[2]->setText(QString::fromLocal8Bit("工件检测"));
            break;
        case 1:
            sub2Button[0]->setText(QString::fromLocal8Bit("编辑"));
            editStack->setCurrentIndex(12);
            break;
        case 2:
            sub2Button[0]->setText(QString::fromLocal8Bit("输入"));
            sub2Button[1]->setText(QString::fromLocal8Bit("工件检测"));
            break;
        default:
            break;
        }
        break;
    case 2:
        switch(i)
        {
        case 0:
            sub2Button[0]->setText(QString::fromLocal8Bit("浏览"));
            sub2Button[1]->setText(QString::fromLocal8Bit("删除"));
            sub2Button[2]->setText(QString::fromLocal8Bit("拷贝"));
            sub2Button[3]->setText(QString::fromLocal8Bit("重命名"));
            sub2Button[4]->setText(QString::fromLocal8Bit("合并"));
            sub2Button[5]->setText(QString::fromLocal8Bit("镜像"));
            break;
        case 1:
            sub2Button[0]->setText(QString::fromLocal8Bit("段开始"));
            sub2Button[1]->setText(QString::fromLocal8Bit("段结束"));
            sub2Button[2]->setText(QString::fromLocal8Bit("删除段"));
            break;
        case 2:
            sub2Button[0]->setText(QString::fromLocal8Bit("载入"));
            break;
        case 3:
            pTE_GCode_JumpTo(-1);
            break;
        case 4:
            showTrail();

            break;
        default:
            break;
        }
        break;
    case 3:
        switch(i)
        {
        case 0:
            sub2Button[0]->setText(QString::fromLocal8Bit("载入"));
            break;
        case 1:
            sub2Button[0]->setText(QString::fromLocal8Bit("单段"));
            sub2Button[1]->setText(QString::fromLocal8Bit("连续"));
            break;
        case 2:
            break;
        default:
            break;
        }
        break;
    case 4:
        switch(i)
        {
        case 0:
            mainStack->setCurrentIndex(4);
            sub2Button[0]->setText(QString::fromLocal8Bit("全局"));
            sub2Button[1]->setText(QString::fromLocal8Bit("CCD"));
            sub2Button[2]->setText(QString::fromLocal8Bit("图像"));
            sub2Button[3]->setText(QString::fromLocal8Bit("坐标"));
            sub2Button[4]->setText(QString::fromLocal8Bit("程序"));
            sub2Button[5]->setText(QString::fromLocal8Bit("放大"));
            sub2Button[6]->setText(QString::fromLocal8Bit("实时"));   //realtime drawlines
            sub2Button[7]->setText(QString::fromLocal8Bit("对刀"));
            break;
        case 1:
            mainStack->setCurrentIndex(3);
            sub2Button[0]->setText(QString::fromLocal8Bit("历史报警"));
            sub2Button[1]->setText(QString::fromLocal8Bit("时间"));
            break;
        case 2:
            sub2Button[0]->setText(QString::fromLocal8Bit("零点偏置"));
            sub2Button[1]->setText(QString::fromLocal8Bit("刀具偏置"));
            sub2Button[2]->setText(QString::fromLocal8Bit("修正补偿"));
            sub2Button[3]->setText(QString::fromLocal8Bit("滑板往复"));
            break;
        case 3:
            sub2Button[0]->setText(QString::fromLocal8Bit("坐标系"));
            sub2Button[1]->setText(QString::fromLocal8Bit("刀具"));
            sub2Button[2]->setText(QString::fromLocal8Bit("补偿"));
            break;
        default:
            break;
        }
        break;
    case 5:
        switch(i)
        {
        case 0:
            sub2Button[0]->setText(QString::fromLocal8Bit("系统设定"));
            sub2Button[1]->setText(QString::fromLocal8Bit("机械设定"));
            sub2Button[2]->setText(QString::fromLocal8Bit("I/O读写"));
            sub2Button[3]->setText(QString::fromLocal8Bit("轴设定"));
            sub2Button[4]->setText(QString::fromLocal8Bit("调试终端"));
            sub2Button[5]->setText(QString::fromLocal8Bit("丝杠补偿"));
            break;
        case 1:

            break;
        case 2:
            sub2Button[0]->setText(QString::fromLocal8Bit("配置"));
            sub2Button[1]->setText(QString::fromLocal8Bit("内存测试"));
            sub2Button[2]->setText(QString::fromLocal8Bit("硬件测试"));
            sub2Button[3]->setText(QString::fromLocal8Bit("代码测试"));
            sub2Button[4]->setText(QString::fromLocal8Bit("状态测试"));
            sub2Button[5]->setText(QString::fromLocal8Bit("调试终端"));
            sub2Button[6]->setText(QString::fromLocal8Bit("机床振动"));
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    current_ButtonID_sub=i;
}

void MainWindow::pressed_sub2ButtonGroup(int i)
{
    for(int j=0;j<=7;j++)
    {
        button_unpressed(sub2Button[j]);
    }
    button_pressed(sub2Button[i]);

    switch(current_ButtonID_main)
    {
    case 0:
        switch (current_ButtonID_sub) {
        case 0:
            switch (i) {
            case 0:
                editStack->setCurrentIndex(3);
                break;
            case 1:
                editStack->setCurrentIndex(4);
                break;
//            case 2:
//                editStack->setCurrentIndex(3);
                //                break;
                //            case 3:
                //                editStack->setCurrentIndex(4);
                //                break;
            case 4:
                editStack->setCurrentIndex(14);
            default:
                break;
            }
            break;
        case 1:
            switch (i) {
            case 0:
                editStack->setCurrentIndex(5);
                break;
            default:
                break;
            }
            break;
        case 2:
            switch (i) {
            case 0:
                editStack->setCurrentIndex(6);
                break;
            case 1:
                editStack->setCurrentIndex(7);
                break;
            case 2:
                editStack->setCurrentIndex(8);
                break;
            case 3:
                editStack->setCurrentIndex(9);
                break;
            case 4:
                editStack->setCurrentIndex(10);
                break;
            case 5:
                editStack->setCurrentIndex(11);
                break;
            default:
                break;
            }
            break;
        case 3:
            switch (i) {
            case 0:

                break;
            case 1:

                break;
            default:
                break;
            }
        default:
            break;
        }

        break;
    case 1:
        switch (current_ButtonID_sub) {
        case 0:
            switch (i) {
            case 0:
                editStack->setCurrentIndex(7);
                break;
            case 1:
                editStack->setCurrentIndex(6);
                break;
            default:
                break;
            }
            break;
        case 1:
            switch (i) {
            case 0:
                break;
            default:
                break;
            }
            break;
        case 2:
            switch (i) {
            case 0:
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    case 2:
        switch (current_ButtonID_sub) {
        case 0:
            switch (i) {
            case 0:
                break;
            default:
                break;
            }
            break;
        case 1:
            switch (i) {
            case 0:
                break;
            default:
                break;
            }
            break;
        case 2:
            switch (i) {
            case 0:
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    case 3:
        switch (current_ButtonID_sub) {
        case 0:
            switch (i) {
            case 0:
                break;
            default:
                break;
            }
            break;
        case 1:
            switch (i) {
            case 0:
                break;
            case 1: //311
            {
                bool ok(false);
                emit call_Trio_run_program(&ok,"START_UP");
                editStack->setCurrentIndex(13);

            }
                break;
            default:
                break;
            }
            break;
        case 2:
            switch (i) {
            case 0:
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    case 4:
        switch (current_ButtonID_sub) {
        case 0:
            switch(i){
            case 5:{
            }
                break;
            case 6:{
                mainStack->setCurrentIndex(5);
                //test
                //QPointF mpos_UVt;
                //mpos_UVt.setX(100);
                //mpos_UVt.setY(100);
                //lp->PointF_Vec.append(mpos_UVt);
                //rp->point_vec.append(mpos_UVt);
                //emit return_QPointF(mpos_UVt);
                //lp->storePoints(mpos_UVt);
                //mpos_UVt.setX(200);
                //mpos_UVt.setY(250);
                //lp->PointF_Vec.append(mpos_UVt);
                //emit return_QPointF(mpos_UVt);
                //lp->storePoints(mpos_UVt);
                //rp->point_vec.append(mpos_UVt);
                //rp->show();
            }
                break;
            case 7:{
                if(!tool_Aiming){
                    GL_CCD->pB_ToolAiming_On=true;
                    tool_Aiming=true;
                }
                else{
                    GL_CCD->pB_ToolAiming_On=false;
                    tool_Aiming=false;
                }
            }
                break;
            }
        }
    }
}

void MainWindow::button_pressed(QPushButton *button)
{
    button->setPalette(Palette_Pressed);
}

void MainWindow::button_unpressed(QPushButton *button)
{
    button->setPalette(Palette_Unpressed);
}

void MainWindow::clear_button_text(BUTTON_GROUP_TYPE group_type)
{
    switch(group_type)
    {
    case MAIN:
        mainButton[0]->setText(QString(" "));
        mainButton[1]->setText(QString(" "));
        mainButton[2]->setText(QString(" "));
        mainButton[3]->setText(QString(" "));
        mainButton[4]->setText(QString(" "));
        mainButton[5]->setText(QString(" "));
        button_unpressed(mainButton[0]);
        button_unpressed(mainButton[1]);
        button_unpressed(mainButton[2]);
        button_unpressed(mainButton[3]);
        button_unpressed(mainButton[4]);
        button_unpressed(mainButton[5]);
        break;
    case SUB:
        subButton[0]->setText(QString(" "));
        subButton[1]->setText(QString(" "));
        subButton[2]->setText(QString(" "));
        subButton[3]->setText(QString(" "));
        subButton[4]->setText(QString(" "));
        button_unpressed(subButton[0]);
        button_unpressed(subButton[1]);
        button_unpressed(subButton[2]);
        button_unpressed(subButton[3]);
        button_unpressed(subButton[4]);
        break;
    case SUB2:
        sub2Button[0]->setText(QString(" "));
        sub2Button[1]->setText(QString(" "));
        sub2Button[2]->setText(QString(" "));
        sub2Button[3]->setText(QString(" "));
        sub2Button[4]->setText(QString(" "));
        sub2Button[5]->setText(QString(" "));
        sub2Button[6]->setText(QString(" "));
        sub2Button[7]->setText(QString(" "));
        button_unpressed(sub2Button[0]);
        button_unpressed(sub2Button[1]);
        button_unpressed(sub2Button[2]);
        button_unpressed(sub2Button[3]);
        button_unpressed(sub2Button[4]);
        button_unpressed(sub2Button[5]);
        button_unpressed(sub2Button[6]);
        button_unpressed(sub2Button[7]);
        break;
    case ALL_SUB:
        subButton[0]->setText(QString(" "));
        subButton[1]->setText(QString(" "));
        subButton[2]->setText(QString(" "));
        subButton[3]->setText(QString(" "));
        subButton[4]->setText(QString(" "));
        sub2Button[0]->setText(QString(" "));
        sub2Button[1]->setText(QString(" "));
        sub2Button[2]->setText(QString(" "));
        sub2Button[3]->setText(QString(" "));
        sub2Button[4]->setText(QString(" "));
        sub2Button[5]->setText(QString(" "));
        sub2Button[6]->setText(QString(" "));
        sub2Button[7]->setText(QString(" "));
        button_unpressed(subButton[0]);
        button_unpressed(subButton[1]);
        button_unpressed(subButton[2]);
        button_unpressed(subButton[3]);
        button_unpressed(subButton[4]);
        button_unpressed(sub2Button[0]);
        button_unpressed(sub2Button[1]);
        button_unpressed(sub2Button[2]);
        button_unpressed(sub2Button[3]);
        button_unpressed(sub2Button[4]);
        button_unpressed(sub2Button[5]);
        button_unpressed(sub2Button[6]);
        button_unpressed(sub2Button[7]);
        break;
    case ALL:
        mainButton[0]->setText(QString(" "));
        mainButton[1]->setText(QString(" "));
        mainButton[2]->setText(QString(" "));
        mainButton[3]->setText(QString(" "));
        mainButton[4]->setText(QString(" "));
        mainButton[5]->setText(QString(" "));
        subButton[0]->setText(QString(" "));
        subButton[1]->setText(QString(" "));
        subButton[2]->setText(QString(" "));
        subButton[3]->setText(QString(" "));
        subButton[4]->setText(QString(" "));
        sub2Button[0]->setText(QString(" "));
        sub2Button[1]->setText(QString(" "));
        sub2Button[2]->setText(QString(" "));
        sub2Button[3]->setText(QString(" "));
        sub2Button[4]->setText(QString(" "));
        sub2Button[5]->setText(QString(" "));
        sub2Button[6]->setText(QString(" "));
        sub2Button[7]->setText(QString(" "));
        button_unpressed(mainButton[0]);
        button_unpressed(mainButton[1]);
        button_unpressed(mainButton[2]);
        button_unpressed(mainButton[3]);
        button_unpressed(mainButton[4]);
        button_unpressed(mainButton[5]);
        button_unpressed(subButton[0]);
        button_unpressed(subButton[1]);
        button_unpressed(subButton[2]);
        button_unpressed(subButton[3]);
        button_unpressed(subButton[4]);
        button_unpressed(sub2Button[0]);
        button_unpressed(sub2Button[1]);
        button_unpressed(sub2Button[2]);
        button_unpressed(sub2Button[3]);
        button_unpressed(sub2Button[4]);
        button_unpressed(sub2Button[5]);
        button_unpressed(sub2Button[6]);
        button_unpressed(sub2Button[7]);
        break;
    }
}

QString MainWindow::get_MTYPE_str(int mtype)
{
    QString temp_str=QString::fromLocal8Bit("空闲");
    switch (mtype)
    {
    case 0:
        temp_str=QString::fromLocal8Bit("空闲");
        break;
    case 1:
        temp_str=QString::fromLocal8Bit("直线相对运动");
        break;
    case 2:
        temp_str=QString::fromLocal8Bit("直线绝对运动");
        break;
    case 3:
        temp_str=QString::fromLocal8Bit("螺旋线运动");
        break;
    case 4:
        temp_str=QString::fromLocal8Bit("圆弧运动");
        break;
    default:
        temp_str=QString::fromLocal8Bit("其余运动");
        break;
    }

    return temp_str;
}

void MainWindow::current_time_text_set()
{
    QString str;
    QDateTime current_one=QDateTime::currentDateTime();
    str=current_one.toString(QString("yyyy MM dd hh:mm:ss"));

    ui->Label_date->setText(str);
}

void MainWindow::txtfile_new_built()    //xinjian
{

}

void MainWindow::txtfile_readin()   //duru
{
    //QString fileDir=QFileDialog::getExistingDirectory(this,QString::fromLocal8Bit("请选择目录"),QString::fromLocal8Bit("目录为"));
    //emit cB_Txt_Changed(fileDir);

    QString path = QFileDialog::getOpenFileName(this,
                                                QString::fromLocal8Bit("打开文本文件"),
                                                "D:",   //default
                                                QString::fromLocal8Bit("文本文件(*.txt)"));
    if (path.length() == 0)
    {
        QMessageBox::information(NULL, tr("Path"),
                                 QString::fromLocal8Bit("没有选择任何文本文件。"));
    }
    else
    {
        if(list.isEmpty()==false) list.clear(); //清空list
        QMessageBox::information(NULL, tr("Path"),
                                 QString::fromLocal8Bit("已选择了该文本文件") + path);
    }
    // 记录文件路径

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(NULL, tr("File"),
                                 tr("You can't open the file"));
    }
    else
    {
        int i = 0;
        QString context = NULL;
        while(!file.atEnd())
        {
            QByteArray line = file.readLine();
            context.append(line);
            list << line;
            i = i+1;
        }
        lineCount = i;
        //ui->txtDisplay->setText(context);
        emit label_Txt_Changed(path); //revised
        ui->pTE_GCode->setPlainText(context);
    }
    // 打开并保存文本文件
    dir_of_txt=path;
}

void MainWindow::txtfile_save() //baocun
{
    QString fileName_Str,txt_file_absolute_path;
    //fileName_Str=ui->cB_Txt->currentText();
    fileName_Str=ui->label_Txt->text();
    txt_file_absolute_path=dir_of_txt.absolutePath();

    QFile txt_file(txt_file_absolute_path);
    if (!txt_file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        emit errors_in_runtime(2);
        return;
    }

    QTextStream txt_stream(&txt_file);
    txt_stream<<ui->pTE_GCode->toPlainText();
    txt_file.close();
}

void MainWindow::txtfile_undo() //chexiao
{
    ui->pTE_GCode->undo();
}

void MainWindow::txtfile_grammar_check()    //yufajiancha
{
    if(list.isEmpty())
    {
        chooseChecked = false;
        QMessageBox::information(NULL, tr("Path"),
                                 QString::fromLocal8Bit("没有选择任何文本文件。"));
    }
    if ( !mistakeChecked &&chooseChecked == true )
    {
        for (int i=0;i<lineCount;i++)
        {
            QString mistakeAll = NULL;
            mistakeAll.append(MistakeType_1(i));
            mistakeAll.append(MistakeType_2(i));
            if (mistakeAll != NULL)
            {
                MistakeUi->mistakeLine->addItem(new QListWidgetItem
                                                (QString::number(i+1)));
            }
        }
    }

    this->mistakeChecked = true;

    MistakeUi->show();
}

void MainWindow::txtfile_send_to_trio() //xiazai
{
    //test
    encode_circulation_processing(list);

    QString fileName_Str,txt_file_absolute_path,destination_path;
    //fileName_Str=ui->cB_Txt->currentText();
    fileName_Str=ui->label_Txt->text();
    txt_file_absolute_path=dir_of_txt.absolutePath();
    destination_path="TRANSFER_FILE";       //"TRANSFER_FILE"?
    bool ok(false);
    emit call_Trio_send_txt(&ok,txt_file_absolute_path,destination_path);

    trio_MC664->txt_transfer=true;

    //    if(!trio->TextFileLoader(txt_file_absolute_path,0,QString("TEMP_FILE"),0,0,0,0,0,0))
    //    {
    //       emit errors_in_runtime(3);
    //    }else
    //    {
    //        QString string;
    //        trio->Dir(string);
    //        QMessageBox::about(Q_NULLPTR,"ABOUT",string);
    //    }
}


//*******************About Buttons*******************//
void MainWindow::label_Txt_Content(QString str)
{
    ui->label_Txt->clear();
    QFileInfo fileinfo = QFileInfo(str);
    QString file_name = fileinfo.fileName();
    ui->label_Txt->setText(file_name);

    dir_of_txt = fileinfo.absoluteDir();
}


//void MainWindow::cB_Txt_Dir_Content(QString str)
//{
//    QDir dir(str);
//    if (!dir.exists())
//    {
//        emit errors_in_runtime(1);
//        return;
//    }

//    dir.setFilter(QDir::Files|QDir::NoSymLinks|QDir::Readable|QDir::Writable);
//    dir.setSorting(QDir::Name);

//    QStringList txt_Name_List=dir.entryList();
//    //ui->cB_Txt->clear();
//    foreach(QString str,txt_Name_List)
//    {
//        if ((str.right(4)==".txt")|(str.right(4)==".TXT"))
//        {
//            //ui->cB_Txt->addItem(str);
//            //ui->cB_Txt->insertItem(0, str);
//            //ui->cB_Txt->setCurrentText(str);

//        }
//    }
//    //ui->cB_Txt->setCurrentText("hhh.txt");
//    //qDebug()<<ui->cB_Txt->currentText();
//    dir_of_txt=dir;
//}

//void MainWindow::cB_current_index_changed(QString fileName_Str)
//{
//    QString txt_file_absolute_path=dir_of_txt.absolutePath()+"/"+fileName_Str;
//    QFile txt_file(txt_file_absolute_path);
//    if (!txt_file.open(QIODevice::ReadOnly|QIODevice::Text))
//    {
//        emit errors_in_runtime(2);
//        return;
//    }

//    ui->pTE_GCode->clear();
//    QTextStream txt_stream(&txt_file);
//    const QString all_txt=txt_stream.readAll();
//    ui->pTE_GCode->setPlainText(all_txt);
//    txt_file.close();
//}


void MainWindow::errors_handled(int error_type)
{
    QString error_title,error_content;
    error_title=QString::fromLocal8Bit("发生错误");
    error_content=QString::fromLocal8Bit("未知错误");
    switch (error_type) {
    case 1:
        error_content=QString::fromLocal8Bit("选择的目录不存在！");
        break;
    case 2:
        error_content=QString::fromLocal8Bit("选择的txt文件无法打开！");
        break;
    default:
        break;
    }

    QMessageBox::warning(this,error_title,error_content);
}

void MainWindow::errors_of_trio_handled(int code, QString source, QString disc, QString help)
{
    QString str;
    str.clear();
    str.append("错误代码：").append(QString::number(code)).append("\n");
    str.append("来源：").append(source).append("\n");
    str.append("Disc：").append(disc).append("\n");
    str.append("帮助：").append(help);
    QMessageBox::warning(this,"Trio报错",str);
}

void MainWindow::pB_Connection()
{
    bool ok(false);
    emit call_Trio_connect(&ok);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    qSleep(2000);
    QApplication::restoreOverrideCursor();
    if(ok)
    {
        Label_Connection_Status->setPalette(Palette_Connected);
        Label_Connection_Status->setText(QString::fromLocal8Bit("下位机已连接"));
        Connection_Status_of_Trio=true;
    }
    else
    {
        Label_Connection_Status->setPalette(Palette_Unconnected);
        Label_Connection_Status->setText(QString::fromLocal8Bit("下位机未连接"));
        Connection_Status_of_Trio=false;
    }
}

void MainWindow::request_Trio_parameters_request()
{
    if (trio_MC664->Trio_Is_Open())
    {
        bool *ok=new bool(false);
        emit call_Trio_return_axis_paras(ok);
    }
}

void MainWindow::receive_Trio_axis_paras(bool *ok, QVariant var)
{
    Struct_Trio_Paras paras=var.value<Struct_Trio_Paras>();

    u_current_position=paras.axis_mpos[AXIS_U];
    v_current_position=-paras.axis_mpos[AXIS_V];
//    u_current_position=7.3;
//    v_current_position=12;

    if ((x_position_show-paras.axis_mpos[AXIS_X])<0.0001 && (x_position_show-paras.axis_mpos[AXIS_X])>-0.0001){
        x_position_show=x_position_show;
    }else{
        x_position_show=paras.axis_mpos[AXIS_X];
    }
    if ((y_position_show-paras.axis_mpos[AXIS_Y])<0.0001 && (y_position_show-paras.axis_mpos[AXIS_Y])>-0.0001){
        y_position_show=y_position_show;
    }else{
        y_position_show=paras.axis_mpos[AXIS_Y];
    }
    if ((z_position_show-paras.axis_mpos[AXIS_Z])<0.0001 && (z_position_show-paras.axis_mpos[AXIS_Z])>-0.0001){
        z_position_show=z_position_show;
    }else{
        z_position_show=paras.axis_mpos[AXIS_Z];
    }
    if ((u_position_show-paras.axis_mpos[AXIS_U])<0.0001 && (u_position_show-paras.axis_mpos[AXIS_U])>-0.0001){
        u_position_show=u_position_show;
    }else{
        u_position_show=paras.axis_mpos[AXIS_U];
    }
    if ((v_position_show-paras.axis_mpos[AXIS_V])<0.0001 && (v_position_show-paras.axis_mpos[AXIS_V])>-0.0001){
        v_position_show=v_position_show;
    }else{
        v_position_show=paras.axis_mpos[AXIS_V];
    }

    ui->Label_Mech_Cor_X->setText(QString::number(x_position_show,'f',5));
    ui->Label_Mech_Cor_Y->setText(QString::number(y_position_show,'f',5));
    ui->Label_Mech_Cor_U->setText(QString::number(u_position_show,'f',5));
    ui->Label_Mech_Cor_V->setText(QString::number(v_position_show,'f',5));
    ui->Label_Mech_Cor_Z->setText(QString::number(z_position_show,'f',5));

    ui->Label__Motion_Status_X->setText(get_MTYPE_str((int)paras.axis_mtype[AXIS_X]));
    ui->Label__Motion_Status_Y->setText(get_MTYPE_str((int)paras.axis_mtype[AXIS_Y]));
    ui->Label__Motion_Status_U->setText(get_MTYPE_str((int)paras.axis_mtype[AXIS_U]));
    ui->Label__Motion_Status_V->setText(get_MTYPE_str((int)paras.axis_mtype[AXIS_V]));
    ui->Label__Motion_Status_Z->setText(get_MTYPE_str((int)paras.axis_mtype[AXIS_Z]));

    ui->Label_X_Speed->setText(QString::number(paras.axis_speed[AXIS_X],'g',3));
    ui->Label_Y_Speed->setText(QString::number(paras.axis_speed[AXIS_Y],'g',3));
    ui->Label_U_Speed->setText(QString::number(paras.axis_speed[AXIS_U],'g',3));
    ui->Label_V_Speed->setText(QString::number(paras.axis_speed[AXIS_V],'g',3));
    ui->Label_Z_Speed->setText(QString::number(paras.axis_speed[AXIS_Z],'g',3));

//    if ((x_mspeed_show-paras.axis_mspeed[AXIS_X])<0.01 && (x_mspeed_show-paras.axis_mspeed[AXIS_X])>-0.01){
//        x_mspeed_show=x_mspeed_show;
//    }else{
//        x_mspeed_show=paras.axis_mspeed[AXIS_X];
//    }
//    if ((y_mspeed_show-paras.axis_mspeed[AXIS_Y])<0.01 && (y_mspeed_show-paras.axis_mspeed[AXIS_Y])>-0.01){
//        y_mspeed_show=y_mspeed_show;
//    }else{
//        y_mspeed_show=paras.axis_mspeed[AXIS_Y];
//    }
    if ((z_mspeed_show-paras.axis_mspeed[AXIS_Z])<0.01 && (z_mspeed_show-paras.axis_mspeed[AXIS_Z])>-0.01){
        z_mspeed_show=z_mspeed_show;
    }else{
        z_mspeed_show=paras.axis_mspeed[AXIS_Z];
    }
    if ((u_mspeed_show-paras.axis_mspeed[AXIS_U])<0.01 && (u_mspeed_show-paras.axis_mspeed[AXIS_U])>-0.01){
        u_mspeed_show=u_mspeed_show;
    }else{
        u_mspeed_show=paras.axis_mspeed[AXIS_U];
    }
    if ((v_mspeed_show-paras.axis_mspeed[AXIS_V])<0.1 && (v_mspeed_show-paras.axis_mspeed[AXIS_V])>-0.1){
        v_mspeed_show=v_mspeed_show;
    }else{
        v_mspeed_show=paras.axis_mspeed[AXIS_V];
    }


    ui->Label_X_MSpeed->setText(QString::number(cal_ins_Xspeed(paras.axis_mspeed[AXIS_X]),'g',3));//m
    ui->Label_Y_MSpeed->setText(QString::number(cal_ins_Yspeed(paras.axis_mspeed[AXIS_Y]),'g',3));//m
    ui->Label_U_MSpeed->setText(QString::number(u_mspeed_show,'g',3));
    ui->Label_V_MSpeed->setText(QString::number(v_mspeed_show,'g',3));
    ui->Label_Z_MSpeed->setText(QString::number(z_mspeed_show,'g',3));

    ui->Label_current_velocity->setText(QString::number(paras.uv_current_velocity,'g',3));
    ui->Label_current_z_velocity->setText(QString::number(paras.z_current_velocity,'g',3));
    ui->Label_current_feed_rate->setText(QString::number(paras.uv_feed_rate,'g',3));
    ui->Label_current_loop_rate->setText(QString::number(paras.z_feed_rate,'g',3));


    ui->Label_XY_VPSPEED->setText(QString::number(paras.axis_vpspeed[XY],'g',2));
    ui->Label_UV_VPSPEED->setText(QString::number(paras.axis_vpspeed[UV],'g',2));


    latest_Trio_paras=paras;

    QPointF mpos_UV;
    mpos_UV.setX(paras.axis_mpos[AXIS_V]*MAG_FACTOR);  //exchange UV position
    mpos_UV.setY(paras.axis_mpos[AXIS_U]*MAG_FACTOR);
    emit return_QPointF(mpos_UV);

    if(currentLine!=paras.processing_line)
    {
            processing_line(paras.processing_line);
            currentLine=paras.processing_line;
    }

}

void MainWindow::receive_current_time(QString *str)
{
    ui->Label_date->setText(*str);
    delete str;
}

void MainWindow::receive_captured_image(uchar* img_data)
{
//    QImage img=QImage(img_data,2448,2058,QImage::Format_Indexed8);
//    img=img.scaled(ui->Label_Img->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
//    ui->Label_Img->setPixmap(QPixmap::fromImage(img));
    //默认屏幕宽度
    int default_width(800);

    //创建Mat对象并显示
    Mat image(2058,2448,CV_8U,Scalar(0));
    int nl=image.rows;
    int nc=image.cols;
    for (int j=0;j<nl;j++){
        uchar* data=image.ptr<uchar>(j);
        for (int i=0;i<nc;i++){
            data[i]=*(img_data+j*image.step+i);
        }
    }

    trans_image=image;



    if(photo_ok)
    {
        QString filename=ui->Label_Mech_Cor_Z->text();
        string fileAsSave=filename.toStdString();
        imwrite(fileAsSave+".bmp",image);
        photo_ok=false;
    }

    //颜色转换后画中线
    cvtColor(image,image,CV_GRAY2RGB);
    int row,col;
    row=image.rows;
    col=image.cols;
    line(image,Point(col/2,0),Point(col/2,row-1),Scalar(0,0,255),2);
    line(image,Point(0,row/2),Point(col-1,row/2),Scalar(0,255,0),2);
    line(image,Point(221,1700),Point(436,1700),Scalar(0, 0, 255),2,8);
    ellipse(image,Point(436,1413),Size(287,287),0,32.22,90,Scalar(0, 0, 255),2,8);
    ellipse(image,Point(1224,1909),Size(645,645),0,212.22,327.78,Scalar(0, 0, 255),2,8);
    ellipse(image,Point(2011,1413),Size(287,287),0,90,147.78,Scalar(0, 0, 255),2,8);
    line(image,Point(2011,1700),Point(2226,1700),Scalar(0, 0, 255),2,8);

    std::string str_X_mpos,str_Y_mpos,str_U_mpos,str_V_mpos,str_Z_mpos;

    QString qstr=QString("X:")+QString::number(latest_Trio_paras.axis_mpos[AXIS_X]);
    str_X_mpos=qstr.toStdString();
    qstr=QString("Y:")+QString::number(latest_Trio_paras.axis_mpos[AXIS_Y]);
    str_Y_mpos=qstr.toStdString();
    qstr=QString("U:")+QString::number(latest_Trio_paras.axis_mpos[AXIS_U]);
    str_U_mpos=qstr.toStdString();
    qstr=QString("V:")+QString::number(latest_Trio_paras.axis_mpos[AXIS_V]);
    str_V_mpos=qstr.toStdString();
    qstr=QString("Z:")+QString::number(latest_Trio_paras.axis_mpos[AXIS_Z]);
    str_Z_mpos=qstr.toStdString();

    int interval(100);
    putText(image,str_X_mpos,Point(col*3/4,row/8+interval*0),FONT_HERSHEY_SIMPLEX,3,Scalar(0,0,255),3);
    putText(image,str_Y_mpos,Point(col*3/4,row/8+interval*1),FONT_HERSHEY_SIMPLEX,3,Scalar(0,0,255),3);
    putText(image,str_U_mpos,Point(col*3/4,row/8+interval*2),FONT_HERSHEY_SIMPLEX,3,Scalar(0,0,255),3);
    putText(image,str_V_mpos,Point(col*3/4,row/8+interval*3),FONT_HERSHEY_SIMPLEX,3,Scalar(0,0,255),3);
    putText(image,str_Z_mpos,Point(col*3/4,row/8+interval*4),FONT_HERSHEY_SIMPLEX,3,Scalar(0,0,255),3);

    namedWindow("Current_Img",WINDOW_NORMAL);
    resizeWindow("Current_Img",default_width,(int)(default_width*2058/2448));
    imshow("Current_Img",image);

    //opencv_processing

//    ui->label_4->setPixmap(QPixmap::fromImage(img.scaled(image.cols, image.rows).scaled(ui->label_4->width(), ui->label_4->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
//    image.copyTo(rgb);


    //delete []img_data;
}

void MainWindow::on_rB_CCD_toggled(bool checked)
{
    if(checked)
        emit initialize_ccd();
    else
        ccd->set_output_img();
}

void MainWindow::on_pushButton_15_clicked()
{
    photo_ok=true;
}

void MainWindow::on_pushButton_write_tool_compensation_clicked()
{
    double temp(0);
    bool ok(false);
    temp=ui->lineEdit_tool_compensation->text().toDouble(&ok);
    if ( !ok | temp<0 )
    {
        QMessageBox::warning(Q_NULLPTR,
                             QString::fromLocal8Bit("数值错误"),
                             QString::fromLocal8Bit("输入数值错误，请检查"));
        return;
    }

    trio_MC664->cutter_set=true;
    emit call_Trio_write_VR(VR_tool_compensation_flag,1);
    emit call_Trio_write_VR(VR_tool_compensation_value,temp);
    tool_compensation=temp;

    QMessageBox::information(Q_NULLPTR,
                             QString::fromLocal8Bit("刀补值已写入"),
                             QString::fromLocal8Bit("刀补值已写入，可开始加工"));
    Label_StatusBar.setText(QString::fromLocal8Bit("刀补值大小为")+QString::number(temp,'g',5));
    trio_MC664->cutter_set=false;
}

void MainWindow::on_pB_Start_offset_clicked()
{
    int offset_axis(0);
    offset_axis=ui->cB_offset_axis->currentIndex();
    offset_axis++;
    double offset_distance(0);
    bool ok(false);
    offset_distance=ui->lineEdit_offset_dis->text().toDouble(&ok);
    if(offset_distance==0 | !ok)
    {
        QMessageBox::warning(Q_NULLPTR,
                             QString::fromLocal8Bit("数值错误"),
                             QString::fromLocal8Bit("输入数值错误，请检查"));
        return;
    }

    emit call_Trio_write_VR(VR_offset_axis,offset_axis);
    emit call_Trio_write_VR(VR_offset_dis,offset_distance);
    Sleep(50);
    emit call_Trio_write_VR(VR_flag_ready_to_offset,1);
    Label_StatusBar.setText(QString::fromLocal8Bit("已开始偏置移动，距离为")+QString::number(offset_distance,'g',5));
}

void MainWindow::receive_Trio_txt_transfer_situation(bool ok)
{
    if (ok)
    {
        QMessageBox::information(Q_NULLPTR,
                                 QString::fromLocal8Bit("已发送"),
                                 QString::fromLocal8Bit("G代码文件已发送至Trio"));
    }else
    {
        QMessageBox::warning(Q_NULLPTR,
                             QString::fromLocal8Bit("发送错误"),
                             QString::fromLocal8Bit("G代码文件发送至Trio过程中错误"));
    }
}

QString MainWindow::MistakeType_1(int j)
{
    QString lineCheck;
    lineCheck = list[j];
    if (lineCheck[0] != 'N')
    {
        return QString::fromLocal8Bit("不是以N开头");
    }
    else
    {
        return NULL;
    }
}

QString MainWindow::MistakeType_2(int j)
{
    QString lineCheck;
    lineCheck = list[j];
    int length;
    int i = 0;
    length = lineCheck.size();
    for (int k=1;k<length;k++)
    {
        if (lineCheck[k] == 'N')
        {
            i = i+1;
        }
    }
    if (i != 0)
    {
        return QString::fromLocal8Bit("含多个N");
    }
    else
    {
        return NULL;
    }
}

void MainWindow::lineChoose(int num)
{
    lineSelect = num;
    QString allType = NULL;
    allType.append(MistakeType_1(num));
    if (MistakeType_1(num) != NULL)
    {
        allType.append('\n');
    }
    allType.append(MistakeType_2(num));
    if (MistakeType_2(num) != NULL)
    {
        allType.append('\n');
    }
    MistakeUi->mistakeType->setText(allType);
    MistakeUi->order->setText(list[num]);
}

void MainWindow::OrderRevise(QString orderRevise)
{
    list[lineSelect] = orderRevise;
}

void MainWindow::showTrail()
{
    for(int k=0;k<list.size();k++) {TrackUi->tracklist<<list[k];}
    TrackUi->trackProcessing(TrackUi->pix);
    TrackUi->show();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==ui->openGLWidget_zoomImage && (event->type()==QMouseEvent::MouseMove))
    {
        QMouseEvent *mouseEvent=static_cast<QMouseEvent *>(event);
        setCursor(Qt::CrossCursor);
        QPoint qPos=mouseEvent->globalPos();
        QPoint qPos1=ui->openGLWidget_zoomImage->mapFromGlobal(qPos);
        QRect rect=ui->openGLWidget_zoomImage->contentsRect();
        QPoint qPos2 = qPos1 + rect.topLeft();
        QString posInfo;
        long label_x=qPos2.x();
        long label_y=qPos2.y();
        emit return_OpenGLWidget_Zoom_Coordinate(label_x, label_y);

        posInfo="(x,y)=";
        posInfo+="(";
        posInfo+=QString::number(label_x);//
        posInfo+=",";
        posInfo+=QString::number(label_y);//
        posInfo+=")";

        ui->label_GLWidget_Coordinate->setText(posInfo);
    }
    else if(obj==ui->openGLWidget_zoomImage && (event->type()==QEvent::Leave))
    {
        setCursor(Qt::ArrowCursor);
    }
    return QWidget::eventFilter(obj,event);
}

//void MainWindow::opencv_processing()
//{
//    mat=imread("C:/Users/cxj/Desktop/NCS3/photo_of_grinder.png");//
//    QImage image;
//    cvtColor(mat, rgb, CV_BGR2RGB);//默认为3通道的处理
//    image = QImage((const uchar*)(rgb.data), rgb.cols, rgb.rows, rgb.cols*rgb.channels(), QImage::Format_RGB888);
//    ui->label_4->setPixmap(QPixmap::fromImage(image.scaled(rgb.cols, rgb.rows).scaled(ui->label_4->width(), ui->label_4->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
//}

//void MainWindow::opencv_findcorner()
//{
//    int left=0,right=0;
//    QString left_string,right_string;
//    magnifyingUi->imgleft->clear();
//    magnifyingUi->imgright->clear();
//    for(int i=rgb.rows-1;i>=0;i--)
//    {
//        if(rgb.at<Vec3b>(i,0)[0]>THRESHOLD&&rgb.at<Vec3b>(i,0)[1]>THRESHOLD&&rgb.at<Vec3b>(i,0)[2]>THRESHOLD)
//        {
//            left=i;
//            break;
//        }
//    }
//        left_string="LEFT CORNER_Y:";
//        left_string+=QString::number(left);
//        magnifyingUi->imgleft->append(left_string);
//    for(int i=rgb.rows-1;i>=0;i--)
//    {
//        if(rgb.at<Vec3b>(i,rgb.cols-1)[0]>THRESHOLD&&rgb.at<Vec3b>(i,rgb.cols-1)[1]>THRESHOLD&&rgb.at<Vec3b>(i,rgb.cols-1)[2]>THRESHOLD)
//        {
//            right=i;
//            break;
//        }
//    }
//        right_string="RIGHT CORNER_Y:";
//        right_string+=QString::number(right);
//        magnifyingUi->imgright->append(right_string);
//}

//Zero_Return
void MainWindow::on_pB_Zero_Return_X_clicked()
{
    if(ui->label_Zero_Return_1->text().isEmpty())
        ui->label_Zero_Return_1->setText(QString("X"));
    else {
        if(ui->label_Zero_Return_2->text().isEmpty())
            ui->label_Zero_Return_2->setText(QString("X"));
        else {
            if(ui->label_Zero_Return_3->text().isEmpty())
                ui->label_Zero_Return_3->setText(QString("X"));
            else {
                if(ui->label_Zero_Return_4->text().isEmpty())
                    ui->label_Zero_Return_4->setText(QString("X"));
                else {
                    if(ui->label_Zero_Return_5->text().isEmpty())
                        ui->label_Zero_Return_5->setText(QString("X"));
                }
            }
        }
    }
}

void MainWindow::on_pB_Zero_Return_Y_clicked()
{
    if(ui->label_Zero_Return_1->text().isEmpty())
        ui->label_Zero_Return_1->setText(QString("Y"));
    else {
        if(ui->label_Zero_Return_2->text().isEmpty())
            ui->label_Zero_Return_2->setText(QString("Y"));
        else {
            if(ui->label_Zero_Return_3->text().isEmpty())
                ui->label_Zero_Return_3->setText(QString("Y"));
            else {
                if(ui->label_Zero_Return_4->text().isEmpty())
                    ui->label_Zero_Return_4->setText(QString("Y"));
                else {
                    if(ui->label_Zero_Return_5->text().isEmpty())
                        ui->label_Zero_Return_5->setText(QString("Y"));
                }
            }
        }
    }
}

void MainWindow::on_pB_Zero_Return_U_clicked()
{
    if(ui->label_Zero_Return_1->text().isEmpty())
        ui->label_Zero_Return_1->setText(QString("U"));
    else {
        if(ui->label_Zero_Return_2->text().isEmpty())
            ui->label_Zero_Return_2->setText(QString("U"));
        else {
            if(ui->label_Zero_Return_3->text().isEmpty())
                ui->label_Zero_Return_3->setText(QString("U"));
            else {
                if(ui->label_Zero_Return_4->text().isEmpty())
                    ui->label_Zero_Return_4->setText(QString("U"));
                else {
                    if(ui->label_Zero_Return_5->text().isEmpty())
                        ui->label_Zero_Return_5->setText(QString("U"));
                }
            }
        }
    }
}

void MainWindow::on_pB_Zero_Return_V_clicked()
{
    if(ui->label_Zero_Return_1->text().isEmpty())
        ui->label_Zero_Return_1->setText(QString("V"));
    else {
        if(ui->label_Zero_Return_2->text().isEmpty())
            ui->label_Zero_Return_2->setText(QString("V"));
        else {
            if(ui->label_Zero_Return_3->text().isEmpty())
                ui->label_Zero_Return_3->setText(QString("V"));
            else {
                if(ui->label_Zero_Return_4->text().isEmpty())
                    ui->label_Zero_Return_4->setText(QString("V"));
                else {
                    if(ui->label_Zero_Return_5->text().isEmpty())
                        ui->label_Zero_Return_5->setText(QString("V"));
                }
            }
        }
    }
}

void MainWindow::on_pB_Zero_Return_Z_clicked()
{
    if(ui->label_Zero_Return_1->text().isEmpty())
        ui->label_Zero_Return_1->setText(QString("Z"));
    else {
        if(ui->label_Zero_Return_2->text().isEmpty())
            ui->label_Zero_Return_2->setText(QString("Z"));
        else {
            if(ui->label_Zero_Return_3->text().isEmpty())
                ui->label_Zero_Return_3->setText(QString("Z"));
            else {
                if(ui->label_Zero_Return_4->text().isEmpty())
                    ui->label_Zero_Return_4->setText(QString("Z"));
                else {
                    if(ui->label_Zero_Return_5->text().isEmpty())
                        ui->label_Zero_Return_5->setText(QString("Z"));
                }
            }
        }
    }
}

void MainWindow::on_pB_Zero_Return_Reset_clicked()
{
    ui->label_Zero_Return_1->clear();
    ui->label_Zero_Return_2->clear();
    ui->label_Zero_Return_3->clear();
    ui->label_Zero_Return_4->clear();
    ui->label_Zero_Return_5->clear();
    //Zero_Return_Vec.clear();
}

void MainWindow::on_pB_Zero_Return_Load_clicked()
{
    Zero_Return_Vec.clear();    //clear vector
    if(!ui->label_Zero_Return_1->text().isEmpty())
        Zero_Return_Vec.append(ui->label_Zero_Return_1->text());
    if(!ui->label_Zero_Return_2->text().isEmpty())
        Zero_Return_Vec.append(ui->label_Zero_Return_2->text());
    if(!ui->label_Zero_Return_3->text().isEmpty())
        Zero_Return_Vec.append(ui->label_Zero_Return_3->text());
    if(!ui->label_Zero_Return_4->text().isEmpty())
        Zero_Return_Vec.append(ui->label_Zero_Return_4->text());
    if(!ui->label_Zero_Return_5->text().isEmpty())
        Zero_Return_Vec.append(ui->label_Zero_Return_5->text());
    //debug test
}

//Coodinate_Setting
void MainWindow::on_pB_Coordinate_Setting_Reset_clicked()
{
    ui->lineEdit_Coordinate_Setting_X->clear();
    ui->lineEdit_Coordinate_Setting_Y->clear();
    ui->lineEdit_Coordinate_Setting_U->clear();
    ui->lineEdit_Coordinate_Setting_V->clear();
    ui->lineEdit_Coordinate_Setting_Z->clear();
}

void MainWindow::on_pB_Coordinate_Setting_Load_clicked() //lineedit未输入是否需要警告？
{
    Coordinate_Setting_Vec.clear();
    if(!ui->lineEdit_Coordinate_Setting_X->text().isEmpty()) {
        if(!ui->lineEdit_Coordinate_Setting_X->text().data()->isNumber()) {
            //warning
            QMessageBox::warning(this, QString::fromLocal8Bit("输入错误"), QString::fromLocal8Bit("X坐标为非数字文本，请输入数字"));
            Coordinate_Setting_Vec.clear();
        }
        else
            Coordinate_Setting_Vec.append(ui->lineEdit_Coordinate_Setting_X->text().toDouble());
    }

    if(!ui->lineEdit_Coordinate_Setting_Y->text().isEmpty()) {
        if(!ui->lineEdit_Coordinate_Setting_Y->text().data()->isNumber()) {
            //warning
            QMessageBox::warning(this, QString::fromLocal8Bit("输入错误"), QString::fromLocal8Bit("Y坐标为非数字文本，请输入数字"));
            Coordinate_Setting_Vec.clear();
        }
        else
            Coordinate_Setting_Vec.append(ui->lineEdit_Coordinate_Setting_Y->text().toDouble());
    }

    if(!ui->lineEdit_Coordinate_Setting_U->text().isEmpty()) {
        if(!ui->lineEdit_Coordinate_Setting_U->text().data()->isNumber()) {
            //warning
            QMessageBox::warning(this, QString::fromLocal8Bit("输入错误"), QString::fromLocal8Bit("U坐标为非数字文本，请输入数字"));
            Coordinate_Setting_Vec.clear();
        }
        else
            Coordinate_Setting_Vec.append(ui->lineEdit_Coordinate_Setting_U->text().toDouble());
    }

    if(!ui->lineEdit_Coordinate_Setting_V->text().isEmpty()) {
        if(!ui->lineEdit_Coordinate_Setting_V->text().data()->isNumber()) {
            //warning
            QMessageBox::warning(this, QString::fromLocal8Bit("输入错误"), QString::fromLocal8Bit("V坐标为非数字文本，请输入数字"));
            Coordinate_Setting_Vec.clear();
        }
        else
            Coordinate_Setting_Vec.append(ui->lineEdit_Coordinate_Setting_V->text().toDouble());
    }

    if(!ui->lineEdit_Coordinate_Setting_Z->text().isEmpty()) {
        if(!ui->lineEdit_Coordinate_Setting_Z->text().data()->isNumber()) {
            //warning
            QMessageBox::warning(this, QString::fromLocal8Bit("输入错误"), QString::fromLocal8Bit("Z坐标为非数字文本，请输入数字"));
            Coordinate_Setting_Vec.clear();
        }
        else
            Coordinate_Setting_Vec.append(ui->lineEdit_Coordinate_Setting_Z->text().toDouble());
    }

    //debug test
}

//Jump to the particular line in pTE_GCode
void MainWindow::pTE_GCode_JumpTo(int JumptoNumber)
{
    QTextDocument *doc = new QTextDocument(ui->pTE_GCode->toPlainText());
    int LineNumber = doc->blockCount();
    bool ok;
    if(JumptoNumber==-1)
    {
        JumptoNumber = QInputDialog::getInt(this,QString::fromLocal8Bit("请输入行号"),QString::fromLocal8Bit("跳转到"),0,1,LineNumber,1,&ok);      //the line number that you want to jump to
        if (ok)
        {
            ui->pTE_GCode->setFocus();
            QTextCursor textcursor = ui->pTE_GCode->textCursor();
            //int toPosition = doc->findBlockByNumber(JumptoNumber -1).position();  //jump to the begin of the line
            int toPosition = doc->findBlockByNumber(JumptoNumber).position() -1;    //jump to the end of the line
            textcursor.setPosition(toPosition,QTextCursor::MoveAnchor);
            ui->pTE_GCode->setCursorWidth(5);
            ui->pTE_GCode->setTextCursor(textcursor);
            ui->pTE_GCode->centerCursor();
            //ui->pTE_GCode->ensureCursorVisible();
        }
        button_unpressed(subButton[3]);
    }
    else
    {
            ui->pTE_GCode->setFocus();
            QTextCursor textcursor = ui->pTE_GCode->textCursor();
            int toPosition = doc->findBlockByNumber(JumptoNumber -1).position();  //jump to the begin of the line
            //int toPosition = doc->findBlockByNumber(JumptoNumber -1).position() +doc->findBlockByNumber(JumptoNumber -1).length()-1 ;    //jump to the end of the line
            textcursor.setPosition(toPosition,QTextCursor::MoveAnchor);
            ui->pTE_GCode->setCursorWidth(5);
            ui->pTE_GCode->setTextCursor(textcursor);
            ui->pTE_GCode->centerCursor();
            //ui->pTE_GCode->ensureCursorVisible();
    }
}

//circulation 【G25】
void MainWindow::encode_circulation_processing(QList<QString> & codelist)
{
    encodelist=codelist;
    size_t count_command = 0;
    float GetNumber(int,QString,int);
    for(int k=codelist.size()-1; k>=0; k--)
    {
        QString codeline = codelist[k];
        bool alreadygotaN(false);
        for(int j=0;j<codeline.size();j++)
        {
            if(codeline[j]=='N')
            {
                if(!alreadygotaN)
                {
                    count_command ++;
                    alreadygotaN=true;
                }
            }
            if(codeline[j]=='G')
            {
                float number = GetNumber(j, codeline, 1);
                if(number == 25)
                {
                    float start_number, end_number, cycle_times;

                    for(int a=j;a<codeline.size();a++)
                    {
                        if(codeline[a]=='N')
                        {
                            bool firstblank(false);
                            start_number = GetNumber(a, codeline, 2);   //type 1 for 【N1 2 3】, type 2 for 【N10 20 3】
                            for(int g=a;g<codeline.size();g++)
                            {
                                if(codeline[g]==' '&& !firstblank)
                                {
                                    end_number = GetNumber(g, codeline, 2);  //type 1 for 【N1 2 3】, type 2 for 【N10 20 3】
                                    firstblank=true;
                                }
                                if(codeline[g]==' '&& firstblank)
                                {
                                    cycle_times = GetNumber(g, codeline, 1);
                                }
                            }
                        }
                    }
                    for(int b=0;b<cycle_times;b++)
                    {
                        for(int c=end_number;c>=start_number;c--)
                        {
                            encodelist.insert(k+1, codelist[c-1]);
                        }
                    }
                    encodelist.removeAt(k);
                }
            }
        }
    }
}

//circulation and cursor
inline void MainWindow::processing_line(double linenumber)     //当VR值(linenumber)为1时，光标对应第1行，以此类推
{
    if(1<=linenumber && linenumber<=encodelist.size())  //防止溢出
    {
        linenumber=linenumber - 1;
        QString codeline=encodelist[linenumber];
        float GetNumber(int,QString,int);
        float JumptoNumber = GetNumber(0,codeline, 2);
        pTE_GCode_JumpTo(JumptoNumber);
    }
}

//calculate instantaneous speed
inline double MainWindow::cal_ins_Xspeed(double speed)
{
    double return_speed;
    ins_Xspeed_vec.append(speed);
    if(ins_Xspeed_vec.size()==2)
    {
        return_speed=(ins_Xspeed_vec.last()-ins_Xspeed_vec.first())/timer->interval()*1000;   //timer->interval()*1000 to second
        ins_Xspeed_vec.removeFirst();
        return return_speed;
    }
    else return 0;
}

inline double MainWindow::cal_ins_Yspeed(double speed)
{
    double return_speed;
    ins_Yspeed_vec.append(speed);
    if(ins_Yspeed_vec.size()==2)
    {
        return_speed=(ins_Yspeed_vec.last()-ins_Yspeed_vec.first())/timer->interval()*1000;   //timer->interval()*1000 to second
        ins_Yspeed_vec.removeFirst();
        return return_speed;
    }
    else {
        return 0;
    }
}

void MainWindow::on_tableWidget_28_itemClicked(QTableWidgetItem *item)
{
    ui->lineEdit_tool_compensation->setText(item->text());
}

//UI界面补偿开始按钮
void MainWindow::on_radioButton_toggled(bool checked)
{
    QTimer *t=timer1;
    qDebug()<<"IN";
    if(checked)
    {
        t->start(30);
        connect(t,SIGNAL(timeout()),this,SLOT(start_ImageDataTrans()));
        connect(t,SIGNAL(timeout()),this,SLOT(start_PositionDataTrans()));
        trio_MC664->write_VR(49,0);
        qDebug()<<'M';
    }
    else
    {
        t->stop();
        disconnect(t,SIGNAL(timeout()),this,SLOT(start_ImageDataTrans()));
        disconnect(t,SIGNAL(timeout()),this,SLOT(start_PositionDataTrans()));
        trio_MC664->write_VR(49,1);
        qDebug()<<"N";

    }
}


//补偿条件判断，获取U、V轴位置
void MainWindow::start_PositionDataTrans()
{
    if((ui->Label_Z_MSpeed->text().toFloat()>0)){
        grab_PosNum=0;
    }else{
        grab_PosNum=grab_PosNum;
    }

    if((ui->Label_Mech_Cor_Z->text().toFloat()<-15)&&(ui->Label_Mech_Cor_Z->text().toFloat()>-30) && (ui->Label_Z_MSpeed->text().toFloat()<0)&&(grab_PosNum<1)){
        grab_UVPos=true;
        grab_PosNum=grab_PosNum+1;
    }else{
        grab_UVPos=false;
    }

    if(grab_UVPos==true){
        emit start_grabuv();
        grab_UVPos=false;
    }
}

//获取U、V轴的当前位置
void MainWindow::return_uv(){
    u=u_current_position;
    v=v_current_position;
    if(u>=1 && u<=15){
        emit Posdata_trans(u,v,tool_compensation);
    }
}

//补偿条件判断，进行图像处理
void MainWindow::start_ImageDataTrans(){
    if((ui->Label_Z_MSpeed->text().toFloat()>0)){
        grab_ImgNum=0;
    }else{
        grab_ImgNum=grab_ImgNum;
    }

    if((ui->Label_Mech_Cor_Z->text().toFloat()<-55)&&(ui->Label_Mech_Cor_Z->text().toFloat()>-70) && (ui->Label_Z_MSpeed->text().toFloat()<0)&&(grab_ImgNum<1)){
        grab_pic=true;
        grab_ImgNum=grab_ImgNum+1;
    }else{
        grab_pic=false;
    }

    if(grab_pic==true){
        emit start_opencv();
        grab_pic=false;
    }
}

void MainWindow::opencv_parameters_trans(){
//    trans_image=imread("14.bmp");
    if(u>=1 && u<=15){
        emit Imgdata_trans(trans_image);
    }
}

//设置刀补半径
void MainWindow::compensation_value_set(bool flag,float u_value,float v_value)
{
    if (flag==true)
    {
      trio_MC664->write_VR(39,1);
      double a=double(u_value);
      trio_MC664->write_VR(54,a);
      double b=double(v_value);
      trio_MC664->write_VR(55,b);

    }else{
      trio_MC664->write_VR(39,0);
    }
}

void MainWindow::on_pushButton_zoom_clicked()
{
    mainStack->setCurrentIndex(6);
    GL_CCD->transmit_On=true;
    ui->openGLWidget_zoomImage->setMouseTracking(true);
    ui->openGLWidget_zoomImage->installEventFilter(this);
}

void MainWindow::on_pushButton_backtoCCD_clicked()
{
    mainStack->setCurrentIndex(4);
    ui->openGLWidget_zoomImage->setMouseTracking(false);
}

//void MainWindow::receive_Origin_Coordinate(int x, int y)
//{
//    QString posInfo;
//    posInfo="(x,y)=";
//    posInfo+="(";
//    posInfo+=QString::number(x);//
//    posInfo+=",";
//    posInfo+=QString::number(y);//
//    posInfo+=")";
//    ui->label_Origin_Coordinate->setText(posInfo);
//}

void MainWindow::receive_Slider_Value(int value)
{
    ui->label_Zoom_Factor->setText(QString::number(value));
    GL_zoom->zoom_factor=double(value)/100;
    GL_zoom->max_zoom_num=log(double(10)/2048)/log(GL_zoom->zoom_factor)+1;
    //qDebug()<<GL_zoom->zoom_factor;
    //qDebug()<<GL_zoom->max_zoom_num;
}


