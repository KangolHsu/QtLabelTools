
/* *************************************************************** */
/* mainwindow.h                          */
/* *************************************************************** */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CanvasScene;
class PolygonItem;
class RectangleItem;
class CanvasView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void deleteSelectedShape();
    void changeSelectedShapeLabel();

private slots:
    // 文件操作
    void on_actionOpen_Folder_triggered();
    void on_actionSave_triggered();

    // 列表点击事件
    void on_fileListWidget_itemClicked(QListWidgetItem *item);
    
    // 工具栏动作
    void on_actionNext_Image_triggered();
    void on_actionPrev_Image_triggered();
    void on_actionCreate_Polygon_triggered(bool checked);
    void on_actionCreate_Rectangle_triggered(bool checked);
    
    // 自定义槽函数
    void handlePolygonFinished(PolygonItem* item);
    void handleRectangleFinished(RectangleItem* item);
    void handleSelectionChanged();

    // 新增的槽函数，用于处理标签的增删
    void on_addLabelButton_clicked();
    void on_deleteLabelButton_clicked();
    void on_shapeListWidget_customContextMenuRequested(const QPoint &pos);


private:
    void loadDirectory(const QString& path);
    void loadImage(const QString& imagePath);
    void saveAnnotations(const QString& imagePath);
    void loadAnnotations(const QString& imagePath);
    void populateLabels();
    void updateShapeList();

    Ui::MainWindow *ui;
    CanvasView* view;
    CanvasScene* scene;
    
    QString currentDirectory;
    QStringList imageFiles;
    int currentFileIndex = -1;
};
#endif // MAINWINDOW_H
