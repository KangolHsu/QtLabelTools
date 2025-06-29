
/* *************************************************************** */
/* mainwindow.cpp                          */
/* *************************************************************** */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvasview.h"
#include "canvasscene.h"
#include "polygonitem.h"
#include "rectangleitem.h"

#include <QFileDialog>
#include <QDir>
#include <QGraphicsPixmapItem>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QBuffer>
#include <QFileInfo>
#include <QInputDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Detach the old graphicsView from the layout and delete it
    ui->horizontalLayout->removeWidget(ui->graphicsView);
    ui->graphicsView->setParent(nullptr);
    delete ui->graphicsView;

    // Create and add the new CanvasView
    view = new CanvasView(this);
    ui->horizontalLayout->addWidget(view);
    
    scene = new CanvasScene(this);
    view->setScene(scene);
    
    populateLabels();
    
    connect(scene, &CanvasScene::polygonFinished, this, &MainWindow::handlePolygonFinished);
    connect(scene, &CanvasScene::rectangleFinished, this, &MainWindow::handleRectangleFinished);
    connect(scene, &QGraphicsScene::selectionChanged, this, &MainWindow::handleSelectionChanged);

    ui->shapeListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->shapeListWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::on_shapeListWidget_customContextMenuRequested);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::populateLabels()
{
    ui->labelListWidget->addItems({"cat", "dog", "person", "car", "tree"});
    ui->labelListWidget->setCurrentRow(0);
}

void MainWindow::on_addLabelButton_clicked()
{
    bool ok;
    QString newLabel = QInputDialog::getText(this, "新增标签", "请输入新的标签名称:", QLineEdit::Normal, "", &ok);

    if (ok && !newLabel.isEmpty()) {
        // 检查标签是否已存在
        QList<QListWidgetItem*> foundItems = ui->labelListWidget->findItems(newLabel, Qt::MatchExactly);
        if (foundItems.isEmpty()) {
            ui->labelListWidget->addItem(newLabel);
            ui->labelListWidget->setCurrentRow(ui->labelListWidget->count() - 1); // 自动选中新标签
        } else {
            statusBar()->showMessage("错误：该标签已存在！", 3000);
        }
    }
}

void MainWindow::on_deleteLabelButton_clicked()
{
    QListWidgetItem* selectedItem = ui->labelListWidget->currentItem();
    if (selectedItem) {
        int row = ui->labelListWidget->row(selectedItem);
        delete ui->labelListWidget->takeItem(row);
    } else {
        statusBar()->showMessage("请先在列表中选择一个要删除的标签。", 3000);
    }
}


void MainWindow::on_actionOpen_Folder_triggered()
{
    QString dir = QFileDialog::getExistingDirectory(this, "打开文件夹", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        loadDirectory(dir);
    }
}

void MainWindow::loadDirectory(const QString &path)
{
    currentDirectory = path;
    QDir dir(path);
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp";
    imageFiles = dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot);

    ui->fileListWidget->clear();
    ui->fileListWidget->addItems(imageFiles);

    if (!imageFiles.isEmpty()) {
        currentFileIndex = 0;
        ui->fileListWidget->setCurrentRow(currentFileIndex);
        loadImage(QDir(currentDirectory).filePath(imageFiles[currentFileIndex]));
    }
}

void MainWindow::loadImage(const QString &imagePath)
{
    QList<QGraphicsItem*> items = scene->items();
    for(QGraphicsItem* item : items){
        scene->removeItem(item);
        delete item;
    }
    ui->shapeListWidget->clear();

    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        statusBar()->showMessage("错误：无法加载图片 " + imagePath, 3000);
        return;
    }
    
    scene->addPixmap(pixmap);
    scene->setSceneRect(pixmap.rect());
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    statusBar()->showMessage("已加载图片: " + imagePath, 3000);

    QString jsonPath = imagePath.left(imagePath.lastIndexOf('.')) + ".json";
    loadAnnotations(jsonPath);
}


void MainWindow::on_fileListWidget_itemClicked(QListWidgetItem *item)
{
    currentFileIndex = ui->fileListWidget->row(item);
    loadImage(QDir(currentDirectory).filePath(imageFiles[currentFileIndex]));
}


void MainWindow::on_actionSave_triggered()
{
    if (currentFileIndex != -1) {
        saveAnnotations(QDir(currentDirectory).filePath(imageFiles[currentFileIndex]));
    }
}

void MainWindow::saveAnnotations(const QString& imagePath)
{
    QJsonObject rootObj;
    rootObj["version"] = "5.4.1";
    rootObj["flags"] = QJsonObject();

    QJsonArray shapesArray;
    for (QGraphicsItem *item : scene->items()) {
        if (auto polygonItem = dynamic_cast<PolygonItem*>(item)) {
            QJsonObject shapeObj;
            shapeObj["label"] = polygonItem->getLabel();
            shapeObj["group_id"] = QJsonValue::Null;
            shapeObj["shape_type"] = "polygon";
            shapeObj["flags"] = QJsonObject();
            
            QJsonArray pointsArray;
            for (const QPointF &point : polygonItem->polygon()) {
                QJsonArray pointArray;
                pointArray.append(point.x());
                pointArray.append(point.y());
                pointsArray.append(pointArray);
            }
            shapeObj["points"] = pointsArray;
            shapesArray.append(shapeObj);
        } else if (auto rectangleItem = dynamic_cast<RectangleItem*>(item)) {
            QJsonObject shapeObj;
            shapeObj["label"] = rectangleItem->getLabel();
            shapeObj["group_id"] = QJsonValue::Null;
            shapeObj["shape_type"] = "rectangle";
            shapeObj["flags"] = QJsonObject();

            QJsonArray pointsArray;
            QRectF rect = rectangleItem->rect();
            pointsArray.append(QJsonArray{rect.left(), rect.top()});
            pointsArray.append(QJsonArray{rect.right(), rect.bottom()});
            shapeObj["points"] = pointsArray;
            shapesArray.append(shapeObj);
        }
    }
    rootObj["shapes"] = shapesArray;

    rootObj["imagePath"] = QFileInfo(imagePath).fileName();
    
    QPixmap pixmap(imagePath);
    rootObj["imageHeight"] = pixmap.height();
    rootObj["imageWidth"] = pixmap.width();

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG"); 
    rootObj["imageData"] = QString::fromLatin1(byteArray.toBase64());
    
    QJsonDocument doc(rootObj);
    QString savePath = imagePath.left(imagePath.lastIndexOf('.')) + ".json";
    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        statusBar()->showMessage("标注已保存: " + savePath, 3000);
    } else {
        statusBar()->showMessage("错误：无法保存标注文件 " + savePath, 3000);
    }
}

void MainWindow::loadAnnotations(const QString &jsonPath)
{
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootObj = doc.object();
    QJsonArray shapesArray = rootObj["shapes"].toArray();

    for (const QJsonValue &value : shapesArray) {
        QJsonObject shapeObj = value.toObject();
        if (shapeObj["shape_type"] == "polygon") {
            QPolygonF polygon;
            QJsonArray pointsArray = shapeObj["points"].toArray();
            for (const QJsonValue &pointValue : pointsArray) {
                QJsonArray pointArray = pointValue.toArray();
                polygon << QPointF(pointArray[0].toDouble(), pointArray[1].toDouble());
            }
            
            auto polygonItem = new PolygonItem(polygon);
            polygonItem->setLabel(shapeObj["label"].toString());
            polygonItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
            scene->addItem(polygonItem);
        } else if (shapeObj["shape_type"] == "rectangle") {
            QJsonArray pointsArray = shapeObj["points"].toArray();
            QPointF topLeft(pointsArray[0].toArray()[0].toDouble(), pointsArray[0].toArray()[1].toDouble());
            QPointF bottomRight(pointsArray[1].toArray()[0].toDouble(), pointsArray[1].toArray()[1].toDouble());
            QRectF rect(topLeft, bottomRight);

            auto rectangleItem = new RectangleItem(rect);
            rectangleItem->setLabel(shapeObj["label"].toString());
            rectangleItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
            scene->addItem(rectangleItem);
        }
    }
    updateShapeList();
}


void MainWindow::on_actionNext_Image_triggered()
{
    if (currentFileIndex < imageFiles.size() - 1) {
        currentFileIndex++;
        ui->fileListWidget->setCurrentRow(currentFileIndex);
        loadImage(QDir(currentDirectory).filePath(imageFiles[currentFileIndex]));
    }
}

void MainWindow::on_actionPrev_Image_triggered()
{
    if (currentFileIndex > 0) {
        currentFileIndex--;
        ui->fileListWidget->setCurrentRow(currentFileIndex);
        loadImage(QDir(currentDirectory).filePath(imageFiles[currentFileIndex]));
    }
}

void MainWindow::on_actionCreate_Polygon_triggered(bool checked)
{
    if(checked) {
        if(ui->labelListWidget->currentItem()){
            scene->setCurrentLabel(ui->labelListWidget->currentItem()->text());
            scene->setMode(CanvasScene::DrawPolygon);
            statusBar()->showMessage("模式：绘制多边形。单击添加点，双击完成。", 0);
        } else {
            statusBar()->showMessage("请先在右侧选择一个标签！", 3000);
            ui->actionCreate_Polygon->setChecked(false);
        }
    } else {
        scene->setMode(CanvasScene::NoMode);
        statusBar()->clearMessage();
    }
}

void MainWindow::on_actionCreate_Rectangle_triggered(bool checked)
{
    if(checked) {
        if(ui->labelListWidget->currentItem()){
            scene->setCurrentLabel(ui->labelListWidget->currentItem()->text());
            scene->setMode(CanvasScene::DrawRectangle);
            statusBar()->showMessage("模式：绘制矩形。按住并拖动鼠标，双击完成。", 0);
        } else {
            statusBar()->showMessage("请先在右侧选择一个标签！", 3000);
            ui->actionCreate_Rectangle->setChecked(false);
        }
    } else {
        scene->setMode(CanvasScene::NoMode);
        statusBar()->clearMessage();
    }
}

void MainWindow::handlePolygonFinished(PolygonItem* item)
{
    updateShapeList();
}

void MainWindow::handleRectangleFinished(RectangleItem* item)
{
    updateShapeList();
}

void MainWindow::handleSelectionChanged()
{
    updateShapeList(); 
}

void MainWindow::deleteSelectedShape()
{
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    if(selectedItems.isEmpty()){
        return;
    }
    
    for(QGraphicsItem* item : selectedItems){
        scene->removeItem(item);
        delete item;
    }
    updateShapeList();
}

void MainWindow::changeSelectedShapeLabel()
{
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    if(selectedItems.size() != 1) return;

    QGraphicsItem* item = selectedItems.first();
    PolygonItem* polygonItem = dynamic_cast<PolygonItem*>(item);
    RectangleItem* rectangleItem = dynamic_cast<RectangleItem*>(item);

    if(!polygonItem && !rectangleItem) return;

    QStringList labels;
    for(int i=0; i<ui->labelListWidget->count(); ++i){
        labels << ui->labelListWidget->item(i)->text();
    }

    bool ok;
    QString newLabel = QInputDialog::getItem(this, "修改标签", "选择新标签:", labels, 0, false, &ok);

    if(ok && !newLabel.isEmpty()){
        if (polygonItem) {
            polygonItem->setLabel(newLabel);
        } else if (rectangleItem) {
            rectangleItem->setLabel(newLabel);
        }
        item->update();
        updateShapeList();
    }
}

void MainWindow::on_shapeListWidget_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem* item = ui->shapeListWidget->itemAt(pos);
    if (item) {
        QMenu menu(this);
        QAction *editAction = menu.addAction("编辑标签");
        QAction *deleteAction = menu.addAction("删除");
        QAction *selectedAction = menu.exec(ui->shapeListWidget->mapToGlobal(pos));
        if (selectedAction == deleteAction) {
            QGraphicsItem* graphicsItem = item->data(Qt::UserRole).value<QGraphicsItem*>();
            scene->removeItem(graphicsItem);
            delete graphicsItem;
            updateShapeList();
        } else if (selectedAction == editAction) {
            QGraphicsItem* graphicsItem = item->data(Qt::UserRole).value<QGraphicsItem*>();
            scene->clearSelection();
            graphicsItem->setSelected(true);
            changeSelectedShapeLabel();
        }
    }
}


void MainWindow::updateShapeList()
{
    ui->shapeListWidget->clear();
    for (QGraphicsItem *item : scene->items()) {
        if (auto polygonItem = dynamic_cast<PolygonItem*>(item)) {
             QListWidgetItem* listItem = new QListWidgetItem(
                QString("%1 (Polygon)").arg(polygonItem->getLabel()),
                ui->shapeListWidget
            );
            listItem->setData(Qt::UserRole, QVariant::fromValue<QGraphicsItem*>(polygonItem));
            
            if(polygonItem->isSelected()){
                listItem->setSelected(true);
            }
        } else if (auto rectangleItem = dynamic_cast<RectangleItem*>(item)) {
            QListWidgetItem* listItem = new QListWidgetItem(
                QString("%1 (Rectangle)").arg(rectangleItem->getLabel()),
                ui->shapeListWidget
            );
            listItem->setData(Qt::UserRole, QVariant::fromValue<QGraphicsItem*>(rectangleItem));
            
            if(rectangleItem->isSelected()){
                listItem->setSelected(true);
            }
        }
    }
}
