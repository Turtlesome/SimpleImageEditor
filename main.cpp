#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>
#include <QWidget>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QSlider>
#include <QMouseEvent>
#include <QRect>
#include <QPainter>
#include <QMenu>
#include <QWidgetAction>



bool isCropMode = false;
bool isMirrored = false;
bool zoomMode = false;



class MainWindow : public QMainWindow
{
public:
    MainWindow() {
        this->setWindowTitle("Image Viewer");
        this->resize(1920, 1080);
        this->setStyleSheet("background-color: rgb(255, 255, 255);");
        this->setMouseTracking(true);
        this->setFocusPolicy(Qt::StrongFocus);
        this->setContextMenuPolicy(Qt::CustomContextMenu);
        this->setAcceptDrops(true);
    }
};



class CropLabel : public QLabel {
    QPixmap originalPixmap;
    QRect cropRect;
    QPoint origin;
    bool isSelecting = false;

public:
    CropLabel(QWidget* parent = nullptr) : QLabel(parent) {
        setMouseTracking(true);
        setFocusPolicy(Qt::ClickFocus); // This is needed to receive key events    
    }

    void setOriginalPixmap(const QPixmap& pixmap) {
        originalPixmap = pixmap;
        setPixmap(originalPixmap);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (isCropMode && !originalPixmap.isNull()) {
            origin = event->pos();
            cropRect = QRect(origin, QSize());
            isSelecting = true;
            setFocus(); // Set focus when clicked
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (isCropMode && !originalPixmap.isNull() && event->buttons() & Qt::LeftButton) {
            cropRect = QRect(origin, event->pos()).normalized();
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (isCropMode && !originalPixmap.isNull()) {
            cropRect = QRect(origin, event->pos()).normalized();
            isSelecting = false;
            update();
        }
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (!originalPixmap.isNull() && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && !cropRect.isNull()) {
            setPixmap(originalPixmap.copy(cropRect));
            cropRect = QRect();
            update();
        }
    }

    void paintEvent(QPaintEvent* event) override {
        QLabel::paintEvent(event);
        if (!originalPixmap.isNull() && !cropRect.isNull()) {
            QPainter painter(this);
            painter.setPen(QPen(Qt::red, 2));
            painter.drawRect(cropRect);
        }
    }

};




int main(int argc, char* argv[])
{
    QString fileName;
    QApplication app(argc, argv);
    QPixmap originalPixmap;
    MainWindow window;


    QMenuBar* menuBar = new QMenuBar;
    window.setMenuBar(menuBar);

    QMenu* fileMenu = new QMenu("File");
    QMenu* optionMenu = new QMenu("Options");
    menuBar->addMenu(fileMenu);
    menuBar->addMenu(optionMenu);


    QAction* openAction = new QAction("Open");
    fileMenu->addAction(openAction);

    QAction* clearAction = new QAction("Clear");
    fileMenu->addAction(clearAction);

    QAction* saveAction = new QAction("Save");
    fileMenu->addAction(saveAction);


    QAction* brightnessAction = new QAction("Brightness");
    optionMenu->addAction(brightnessAction);

    QAction* contrastAction = new QAction("Contrast");
    optionMenu->addAction(contrastAction);

    QAction* cropAction = new QAction("Crop");
    optionMenu->addAction(cropAction);

    QAction* mirrorAction = new QAction("Mirror");
    optionMenu->addAction(mirrorAction);

    QAction* resizeAction = new QAction("Resize");
    optionMenu->addAction(resizeAction);

    QAction* rgbAction = new QAction("RGB Sliders");
    optionMenu->addAction(rgbAction);

    QAction* rotateAction = new QAction("Rotate");
    optionMenu->addAction(rotateAction);

    QAction* saturationAction = new QAction("Saturation");
    optionMenu->addAction(saturationAction);

    QAction* zoomAction = new QAction("Zoom");
    optionMenu->addAction(zoomAction);


    CropLabel* imageLabel = new CropLabel;
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(imageLabel);
    QWidget* widget = new QWidget;
    widget->setLayout(layout);
    window.setCentralWidget(widget);



    auto openFile = [&] {
        fileName = QFileDialog::getOpenFileName(&window, "Open Image File", "", "Images (*.png *.jpg)");
        if (!fileName.isEmpty()) {
            originalPixmap.load(fileName);
            imageLabel->setOriginalPixmap(originalPixmap);
            imageLabel->setAlignment(Qt::AlignCenter); // Center the image
        }
    };
    QObject::connect(openAction, &QAction::triggered, openFile);


    auto clearImage = [&] {
        imageLabel->clear();
    };
    QObject::connect(clearAction, &QAction::triggered, clearImage);


    auto saveImage = [&] {
        if (!originalPixmap.isNull()) {
            QImage modifiedImage = originalPixmap.toImage(); 

            QString saveFileName = QFileDialog::getSaveFileName(&window, "Save Image", QDir::homePath(), "Images (*.png *.jpg *.bmp)");

            if (!saveFileName.isEmpty()) {
                if (modifiedImage.save(saveFileName)) {
                    qDebug() << "Image saved successfully.";
                }
                else {
                    qDebug() << "Error: Failed to save the image.";
                }
            }
        }
    };
    QObject::connect(saveAction, &QAction::triggered, saveImage);


    auto adjustBrightness = [&] {
        bool ok;
        int brightness = QInputDialog::getInt(&window, "Adjust Brightness", "Brightness:", 0, -255, 255, 1, &ok);
        if (ok && !originalPixmap.isNull()) {
            QImage image = originalPixmap.toImage();
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    QRgb pixel = image.pixel(x, y);
                    int r = qRed(pixel);
                    int g = qGreen(pixel);
                    int b = qBlue(pixel);
                    int alpha = qAlpha(pixel);
                    r = qBound(0, r + brightness, 255);
                    g = qBound(0, g + brightness, 255);
                    b = qBound(0, b + brightness, 255);
                    image.setPixel(x, y, qRgba(r, g, b, alpha));
                }
            }
            originalPixmap = QPixmap::fromImage(image);
            imageLabel->setPixmap(originalPixmap);
        }
    };
    QObject::connect(brightnessAction, &QAction::triggered, adjustBrightness);


    auto adjustContrast = [&] {
        bool ok;
        double contrast = QInputDialog::getDouble(&window, "Adjust Contrast", "Contrast:", 1.0, 0.0, 10.0, 2, &ok);
        if (ok && !originalPixmap.isNull()) {
            QImage image = originalPixmap.toImage();
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    QRgb pixel = image.pixel(x, y);
                    int r = qRed(pixel);
                    int g = qGreen(pixel);
                    int b = qBlue(pixel);
                    int alpha = qAlpha(pixel);

                    // Adjust contrast
                    r = qBound(0, static_cast<int>(contrast * (r - 127) + 127), 255);
                    g = qBound(0, static_cast<int>(contrast * (g - 127) + 127), 255);
                    b = qBound(0, static_cast<int>(contrast * (b - 127) + 127), 255);

                    // Set the adjusted pixel back to the image
                    image.setPixel(x, y, qRgba(r, g, b, alpha));
                }
            }
            originalPixmap = QPixmap::fromImage(image);
            imageLabel->setPixmap(originalPixmap);
        }
    };
    QObject::connect(contrastAction, &QAction::triggered, adjustContrast);


    QObject::connect(cropAction, &QAction::triggered, [&] {
        isCropMode = true;
        });


    auto mirrorImage = [&] {
        if (!originalPixmap.isNull()) {
            QImage image = originalPixmap.toImage();
            isMirrored = !isMirrored; // Toggle the isMirrored flag
            image = image.mirrored(isMirrored, false); // Mirror the image based on the isMirrored flag
            originalPixmap = QPixmap::fromImage(image);
            imageLabel->setPixmap(originalPixmap);
        }
    };
    QObject::connect(mirrorAction, &QAction::triggered, mirrorImage);


    auto resizeImage = [&] {
        QDialog dialog(&window);
        QFormLayout form(&dialog);

        QSpinBox* widthSpinBox = new QSpinBox(&dialog);
        widthSpinBox->setRange(1, 10000);
        widthSpinBox->setValue(originalPixmap.width());
        form.addRow("Width:", widthSpinBox);

        QSpinBox* heightSpinBox = new QSpinBox(&dialog);
        heightSpinBox->setRange(1, 10000);
        heightSpinBox->setValue(originalPixmap.height());
        form.addRow("Height:", heightSpinBox);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
        form.addRow(&buttonBox);

        QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted && !originalPixmap.isNull()) {
            originalPixmap = originalPixmap.scaled(widthSpinBox->value(), heightSpinBox->value(), Qt::IgnoreAspectRatio);
            imageLabel->setPixmap(originalPixmap);
        }
    };


    auto adjustSaturation = [&] {
        bool ok;
        int saturationShift = QInputDialog::getInt(&window, "Adjust saturation", "saturation Shift:", 0, -180, 180, 1, &ok);
        if (ok && !originalPixmap.isNull()) {
            QImage image = originalPixmap.toImage();
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    QColor color(image.pixel(x, y));
                    int h, s, v, a;
                    color.getHsv(&h, &s, &v, &a);
                    h = (h + saturationShift) % 360;
                    color.setHsv(h, s, v, a);
                    image.setPixelColor(x, y, color);
                }
            }
            originalPixmap = QPixmap::fromImage(image);
            imageLabel->setPixmap(originalPixmap);
        }
    };
    QObject::connect(saturationAction, &QAction::triggered, adjustSaturation);


    auto rotateImage = [&] {
        bool ok;
        int angle = QInputDialog::getInt(&window, "Rotate", "Angle:", 0, -360, 360, 1, &ok);
        if (ok && !originalPixmap.isNull()) {
            QTransform transform;
            transform.rotate(angle);
            QPixmap rotatedPixmap = originalPixmap.transformed(transform, Qt::SmoothTransformation);
            imageLabel->setPixmap(rotatedPixmap);

            originalPixmap = rotatedPixmap;
        }
    };
    QObject::connect(rotateAction, &QAction::triggered, rotateImage);


    auto setZoomMode = [&]  {
        zoomMode = true;
    };
    QObject::connect(zoomAction, &QAction::triggered, setZoomMode);



    window.show();

    return app.exec();
}

