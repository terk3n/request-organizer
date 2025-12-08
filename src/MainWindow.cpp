#include "MainWindow.h"
#include <QByteArray>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QTextEdit>
#include <QXmlStreamReader>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <QUrl>
#include <QPushButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QBuffer>
#include <QImage>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_model(new OrganizerModel(this)) {
  setupUI();
  setupMenuBar();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // Splitter for tree view and request viewer
  m_splitter = new QSplitter(Qt::Vertical, this);

  // Tree view
  m_treeView = new QTreeView(this);
  m_treeView->setModel(m_model);
  m_treeView->setRootIsDecorated(true);
  m_treeView->setAlternatingRowColors(true);
  m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->setEditTriggers(QAbstractItemView::DoubleClicked |
                                QAbstractItemView::SelectedClicked);
    m_treeView->setDragDropMode(QAbstractItemView::DragDrop);
    m_treeView->setDefaultDropAction(Qt::MoveAction);
    m_treeView->setDragEnabled(true);
    m_treeView->setAcceptDrops(true);
    m_treeView->setDropIndicatorShown(true);

  // Font for tree view
  QFont treeFont = m_treeView->font();
  treeFont.setPointSize(10);
  m_treeView->setFont(treeFont);

  // Header
  m_treeView->header()->setStretchLastSection(true);
  m_treeView->header()->setDefaultSectionSize(100);
  m_treeView->setColumnWidth(0, 150);  // Name
  m_treeView->setColumnWidth(1, 150);  // Annotation
  m_treeView->setColumnWidth(2, 120);  // Host
  m_treeView->setColumnWidth(3, 200);  // URL
  m_treeView->setColumnWidth(4, 80);   // Method
  m_treeView->setColumnWidth(5, 150);  // Query
  m_treeView->setColumnWidth(6, 70);   // Status
  m_treeView->setColumnWidth(7, 100);  // Length
  m_treeView->setColumnWidth(8, 100);  // Response Time
  m_treeView->setColumnWidth(9, 150);  // Timestamp
  m_treeView->setColumnWidth(10, 150);  // Details
  m_treeView->setColumnHidden(10, true);  // Hide Details column

  // Request viewer (bottom part) - Request and Response side by side
  QWidget *requestViewerWidget = new QWidget(this);
  QVBoxLayout *viewerLayout = new QVBoxLayout(requestViewerWidget);
  viewerLayout->setContentsMargins(0, 0, 0, 0);

  // Screenshot button bar
  QHBoxLayout *screenshotLayout = new QHBoxLayout();
  screenshotLayout->setContentsMargins(5, 5, 5, 5);
  m_screenshotButton = new QPushButton("View Screenshot", this);
  m_screenshotButton->setEnabled(false);
  screenshotLayout->addWidget(m_screenshotButton);
  screenshotLayout->addStretch();
  QPushButton *addScreenshotButton = new QPushButton("Add Screenshot", this);
  m_removeScreenshotButton = new QPushButton("Remove Screenshot", this);
  m_removeScreenshotButton->setEnabled(false);
  screenshotLayout->addWidget(addScreenshotButton);
  screenshotLayout->addWidget(m_removeScreenshotButton);
  viewerLayout->addLayout(screenshotLayout);

  // Horizontal splitter for Request and Response
  m_requestResponseSplitter = new QSplitter(Qt::Horizontal, this);

  // Request panel (left)
  QWidget *requestWidget = new QWidget(this);
  QVBoxLayout *requestLayout = new QVBoxLayout(requestWidget);
  requestLayout->setContentsMargins(5, 5, 5, 5);
  m_requestLabel = new QLabel("Request:", this);
  m_requestEdit = new QTextEdit(this);
  m_requestEdit->setReadOnly(false);
  QFont requestFont = m_requestEdit->font();
  requestFont.setFamily("Courier New");
  requestFont.setPointSize(10);
  m_requestEdit->setFont(requestFont);
  new HttpSyntaxHighlighter(m_requestEdit->document());
  requestLayout->addWidget(m_requestLabel);
  requestLayout->addWidget(m_requestEdit);

  // Response panel (right)
  QWidget *responseWidget = new QWidget(this);
  QVBoxLayout *responseLayout = new QVBoxLayout(responseWidget);
  responseLayout->setContentsMargins(5, 5, 5, 5);
  m_responseLabel = new QLabel("Response:", this);
  m_responseEdit = new QTextEdit(this);
  m_responseEdit->setReadOnly(false);
  QFont responseFont = m_responseEdit->font();
  responseFont.setFamily("Courier New");
  responseFont.setPointSize(10);
  m_responseEdit->setFont(responseFont);
  new HttpSyntaxHighlighter(m_responseEdit->document());
  responseLayout->addWidget(m_responseLabel);
  responseLayout->addWidget(m_responseEdit);

  m_requestResponseSplitter->addWidget(requestWidget);
  m_requestResponseSplitter->addWidget(responseWidget);
  m_requestResponseSplitter->setStretchFactor(0, 1);
  m_requestResponseSplitter->setStretchFactor(1, 1);
  m_requestResponseSplitter->setSizes({400, 400});

  viewerLayout->addWidget(m_requestResponseSplitter);
  requestViewerWidget->setMinimumHeight(200);

  m_splitter->addWidget(m_treeView);
  m_splitter->addWidget(requestViewerWidget);
  m_splitter->setStretchFactor(0, 2);
  m_splitter->setStretchFactor(1, 1);
  m_splitter->setSizes({400, 200});

  mainLayout->addWidget(m_splitter);

  // Connections
  connect(m_treeView, &QTreeView::doubleClicked, this,
          &MainWindow::onItemDoubleClicked);
  connect(m_treeView, &QTreeView::customContextMenuRequested, this,
          &MainWindow::showContextMenu);
  connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &MainWindow::onSelectionChanged);
  connect(m_requestEdit, &QTextEdit::textChanged, this, &MainWindow::onRequestChanged);
  connect(m_responseEdit, &QTextEdit::textChanged, this, &MainWindow::onResponseChanged);
  connect(m_screenshotButton, &QPushButton::clicked, this, &MainWindow::onScreenshotClicked);
  connect(addScreenshotButton, &QPushButton::clicked, this, &MainWindow::onAddScreenshot);
  connect(m_removeScreenshotButton, &QPushButton::clicked, this, &MainWindow::onRemoveScreenshot);
  
  m_updatingViewer = false;

  setWindowTitle("Request Organizer");
  resize(1000, 800);
}

void MainWindow::setupMenuBar() {
  QMenu *fileMenu = menuBar()->addMenu("File");
  QAction *importAction = fileMenu->addAction("Import Burp XML...");
  fileMenu->addSeparator();
  QAction *exitAction = fileMenu->addAction("Exit");
  connect(importAction, &QAction::triggered, this, &MainWindow::onImportBurp);
  connect(exitAction, &QAction::triggered, this, &QWidget::close);

  QMenu *editMenu = menuBar()->addMenu("Edit");
  QAction *addFolderAction = editMenu->addAction("Add Folder");
  QAction *addRequestAction = editMenu->addAction("Add Request");
  editMenu->addSeparator();
  QAction *deleteAction = editMenu->addAction("Delete");
  QAction *colorAction = editMenu->addAction("Set Color");
  QAction *removeColorAction = editMenu->addAction("Remove Color");
  QAction *editRequestAction = editMenu->addAction("Edit Request");
  QAction *editResponseAction = editMenu->addAction("Edit Response");

  connect(addFolderAction, &QAction::triggered, this, &MainWindow::onAddFolder);
  connect(addRequestAction, &QAction::triggered, this,
          &MainWindow::onAddRequest);
  connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteItem);
  connect(colorAction, &QAction::triggered, this, &MainWindow::onSetColor);
  connect(removeColorAction, &QAction::triggered, this, &MainWindow::onRemoveColor);
  connect(editRequestAction, &QAction::triggered, this,
          &MainWindow::onEditRequest);
  connect(editResponseAction, &QAction::triggered, this,
          &MainWindow::onEditResponse);
}

void MainWindow::onAddFolder() {
  QModelIndex parentIndex = getSelectedIndex();
  bool ok;
  QString name = QInputDialog::getText(
      this, "Add Folder", "Folder name:", QLineEdit::Normal, "New Folder", &ok);

  if (ok && !name.isEmpty()) {
    m_model->addFolder(name, parentIndex);
    m_treeView->expand(parentIndex);
  }
}

void MainWindow::onAddRequest() {
  QModelIndex parentIndex = getSelectedIndex();
  bool ok;
  QString name = QInputDialog::getText(this, "Add Request",
                                       "Request name:", QLineEdit::Normal,
                                       "New Request", &ok);

  if (ok && !name.isEmpty()) {
    QModelIndex newIndex = m_model->addRequest(name, parentIndex);
    m_treeView->expand(parentIndex);
    m_treeView->setCurrentIndex(newIndex);
  }
}

void MainWindow::onDeleteItem() {
  QModelIndex index = getSelectedIndex();
  if (!index.isValid()) {
    QMessageBox::information(this, "Delete",
                             "Please select an item to delete.");
    return;
  }

  int ret = QMessageBox::question(this, "Delete",
                                  "Are you sure you want to delete this item?",
                                  QMessageBox::Yes | QMessageBox::No);
  if (ret == QMessageBox::Yes) {
    m_model->removeRow(index.row(), index.parent());
  }
}

void MainWindow::onSetColor() {
  QModelIndex index = getSelectedIndex();
  if (!index.isValid()) {
    QMessageBox::information(this, "Set Color", "Please select an item.");
    return;
  }

  OrganizerItem *item = m_model->getItem(index);
  if (item->type() == ItemType::Folder) {
    QMessageBox::information(this, "Set Color", "Folders cannot have colors.");
    return;
  }

  QColor color = QColorDialog::getColor(item->color(), this, "Select Color");
  if (color.isValid()) {
    item->setColor(color);
    m_model->saveItem(index);
    m_model->dataChanged(index, index);
  }
}

void MainWindow::onRemoveColor() {
  QModelIndex index = getSelectedIndex();
  if (!index.isValid()) {
    QMessageBox::information(this, "Remove Color", "Please select an item.");
    return;
  }

  OrganizerItem *item = m_model->getItem(index);
  if (item->type() == ItemType::Folder) {
    QMessageBox::information(this, "Remove Color", "Folders cannot have colors.");
    return;
  }

  item->setColor(Qt::white);
  m_model->saveItem(index);
  m_model->dataChanged(index, index);
}

void MainWindow::onAddScreenshot() {
  QModelIndex index = getSelectedIndex();
  if (!index.isValid()) {
    QMessageBox::information(this, "Add Screenshot", "Please select a request item.");
    return;
  }

  OrganizerItem *item = m_model->getItem(index);
  if (item->type() != ItemType::Request) {
    QMessageBox::information(this, "Add Screenshot", "Only request items can have screenshots.");
    return;
  }

  QString fileName = QFileDialog::getOpenFileName(this, "Select Screenshot Image", "", 
                                                  "Image Files (*.png *.jpg *.jpeg *.bmp *.gif)");
  if (fileName.isEmpty()) {
    return;
  }

  QImage image(fileName);
  if (image.isNull()) {
    QMessageBox::warning(this, "Add Screenshot", "Could not load image file.");
    return;
  }

  // Convert to base64
  QByteArray imageData;
  QBuffer buffer(&imageData);
  buffer.open(QIODevice::WriteOnly);
  image.save(&buffer, "PNG");
  QString base64Screenshot = imageData.toBase64();

  item->setScreenshot(base64Screenshot);
  m_model->saveItem(index);
  m_screenshotButton->setEnabled(true);
  m_removeScreenshotButton->setEnabled(true);
}

void MainWindow::onRemoveScreenshot() {
  QModelIndex index = getSelectedIndex();
  if (!index.isValid()) {
    QMessageBox::information(this, "Remove Screenshot", "Please select a request item.");
    return;
  }

  OrganizerItem *item = m_model->getItem(index);
  if (item->type() != ItemType::Request) {
    QMessageBox::information(this, "Remove Screenshot", "Only request items can have screenshots.");
    return;
  }

  item->setScreenshot("");
  m_model->saveItem(index);
  m_screenshotButton->setEnabled(false);
  m_removeScreenshotButton->setEnabled(false);
}

void MainWindow::onScreenshotClicked() {
  QModelIndex index = getSelectedIndex();
  if (!index.isValid()) {
    return;
  }

  OrganizerItem *item = m_model->getItem(index);
  if (item->type() != ItemType::Request || !item->hasScreenshot()) {
    return;
  }

  // Decode screenshot
  QByteArray imageData = QByteArray::fromBase64(item->screenshot().toUtf8());
  QImage image = QImage::fromData(imageData);
  
  if (image.isNull()) {
    QMessageBox::warning(this, "View Screenshot", "Could not load screenshot.");
    return;
  }

  // Show screenshot in a dialog
  QDialog dialog(this);
  dialog.setWindowTitle("Screenshot");
  dialog.setMinimumSize(800, 600);
  
  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  QLabel *imageLabel = new QLabel(&dialog);
  imageLabel->setAlignment(Qt::AlignCenter);
  
  // Scale image to fit dialog while maintaining aspect ratio
  QPixmap pixmap = QPixmap::fromImage(image);
  QSize labelSize(780, 580);
  QPixmap scaledPixmap = pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  imageLabel->setPixmap(scaledPixmap);
  
  layout->addWidget(imageLabel);
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
  layout->addWidget(buttonBox);
  
  connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  
  dialog.exec();
}

void MainWindow::onEditRequest() {
  QModelIndex index = getSelectedIndex();
  if (!index.isValid()) {
    QMessageBox::information(this, "Edit Request",
                             "Please select a request item.");
    return;
  }

  OrganizerItem *item = m_model->getItem(index);
  if (item->type() != ItemType::Request) {
    QMessageBox::information(this, "Edit Request",
                             "Please select a request item.");
    return;
  }

  QString currentRequest = item->request();
  QByteArray decoded = QByteArray::fromBase64(currentRequest.toUtf8());
  QString decodedText = QString::fromUtf8(decoded);

  QDialog dialog(this);
  dialog.setWindowTitle("Edit Request");
  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  QLabel *label = new QLabel("Request (will be saved as base64):", &dialog);
  QTextEdit *textEdit = new QTextEdit(&dialog);
  textEdit->setPlainText(decodedText.isEmpty() ? currentRequest : decodedText);
  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

  layout->addWidget(label);
  layout->addWidget(textEdit);
  layout->addWidget(buttonBox);

  connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  if (dialog.exec() == QDialog::Accepted) {
    QString request = textEdit->toPlainText();
    QString base64Request = request.toUtf8().toBase64();
    m_model->setRequest(index, base64Request);
    updateRequestViewer(index);
  }
}

void MainWindow::onEditResponse() {
  QModelIndex index = getSelectedIndex();
  if (!index.isValid()) {
    QMessageBox::information(this, "Edit Response",
                             "Please select a request item.");
    return;
  }

  OrganizerItem *item = m_model->getItem(index);
  if (item->type() != ItemType::Request) {
    QMessageBox::information(this, "Edit Response",
                             "Please select a request item.");
    return;
  }

  QString currentResponse = item->response();
  QByteArray decoded = QByteArray::fromBase64(currentResponse.toUtf8());
  QString decodedText = QString::fromUtf8(decoded);

  QDialog dialog(this);
  dialog.setWindowTitle("Edit Response");
  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  QLabel *label = new QLabel("Response (will be saved as base64):", &dialog);
  QTextEdit *textEdit = new QTextEdit(&dialog);
  textEdit->setPlainText(decodedText.isEmpty() ? currentResponse : decodedText);
  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

  layout->addWidget(label);
  layout->addWidget(textEdit);
  layout->addWidget(buttonBox);

  connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

  if (dialog.exec() == QDialog::Accepted) {
    QString response = textEdit->toPlainText();
    QString base64Response = response.toUtf8().toBase64();
    m_model->setResponse(index, base64Response);
    updateRequestViewer(index);
  }
}

void MainWindow::onItemDoubleClicked(const QModelIndex &index) {
  if (index.column() == 0) {
    m_treeView->edit(index);
  }
}

void MainWindow::showContextMenu(const QPoint &pos) {
  QModelIndex index = m_treeView->indexAt(pos);
  QMenu contextMenu(this);

  contextMenu.addAction("Add Folder", this, &MainWindow::onAddFolder);
  contextMenu.addAction("Add Request", this, &MainWindow::onAddRequest);
  contextMenu.addSeparator();

  if (index.isValid()) {
    contextMenu.addAction("Delete", this, &MainWindow::onDeleteItem);
    contextMenu.addSeparator();

    OrganizerItem *item = m_model->getItem(index);
    if (item->type() == ItemType::Request) {
      contextMenu.addAction("Set Color", this, &MainWindow::onSetColor);
      contextMenu.addAction("Remove Color", this, &MainWindow::onRemoveColor);
      contextMenu.addSeparator();
      contextMenu.addAction("Edit Request", this, &MainWindow::onEditRequest);
      contextMenu.addAction("Edit Response", this, &MainWindow::onEditResponse);
    }
  }

  contextMenu.exec(m_treeView->mapToGlobal(pos));
}

void MainWindow::onSelectionChanged(const QModelIndex &current,
                                    const QModelIndex &previous) {
  Q_UNUSED(previous);
  updateRequestViewer(current);
}

void MainWindow::updateRequestViewer(const QModelIndex &index) {
  m_updatingViewer = true;
  m_currentIndex = index;
  
  if (!index.isValid()) {
    m_requestEdit->clear();
    m_responseEdit->clear();
    m_updatingViewer = false;
    return;
  }

  OrganizerItem *item = m_model->getItem(index);
  if (item->type() != ItemType::Request) {
    m_requestEdit->clear();
    m_responseEdit->clear();
    m_screenshotButton->setEnabled(false);
    m_removeScreenshotButton->setEnabled(false);
    m_updatingViewer = false;
    return;
  }
  
  // Update screenshot button state
  bool hasScreenshot = item->hasScreenshot();
  m_screenshotButton->setEnabled(hasScreenshot);
  m_removeScreenshotButton->setEnabled(hasScreenshot);

  // Decode and display request
  QString requestBase64 = item->request();
  if (!requestBase64.isEmpty()) {
    QByteArray decoded = QByteArray::fromBase64(requestBase64.toUtf8());
    m_requestEdit->setPlainText(QString::fromUtf8(decoded));
  } else {
    m_requestEdit->setPlainText("");
  }

  // Decode and display response
  QString responseBase64 = item->response();
  if (!responseBase64.isEmpty()) {
    QByteArray decoded = QByteArray::fromBase64(responseBase64.toUtf8());
    m_responseEdit->setPlainText(QString::fromUtf8(decoded));
  } else {
    m_responseEdit->setPlainText("");
  }
  
  m_updatingViewer = false;
}

void MainWindow::onRequestChanged() {
  if (m_updatingViewer || !m_currentIndex.isValid()) {
    return;
  }
  
  OrganizerItem *item = m_model->getItem(m_currentIndex);
  if (item && item->type() == ItemType::Request) {
    QString text = m_requestEdit->toPlainText();
    QString base64Text = text.toUtf8().toBase64();
    m_model->setRequest(m_currentIndex, base64Text);
  }
}

void MainWindow::onResponseChanged() {
  if (m_updatingViewer || !m_currentIndex.isValid()) {
    return;
  }
  
  OrganizerItem *item = m_model->getItem(m_currentIndex);
  if (item && item->type() == ItemType::Request) {
    QString text = m_responseEdit->toPlainText();
    QString base64Text = text.toUtf8().toBase64();
    m_model->setResponse(m_currentIndex, base64Text);
  }
}

QModelIndex MainWindow::getSelectedIndex() {
  QModelIndexList selected = m_treeView->selectionModel()->selectedIndexes();
  if (!selected.isEmpty()) {
    return selected.first();
  }
  return QModelIndex();
}

void MainWindow::onImportBurp() {
  // Dialog to choose import method
  QDialog dialog(this);
  dialog.setWindowTitle("Import Burp XML");
  dialog.setMinimumWidth(400);
  
  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  
  QLabel *label = new QLabel("Choose import method:", &dialog);
  layout->addWidget(label);
  
  QPushButton *fileButton = new QPushButton("Import from File...", &dialog);
  QPushButton *pasteButton = new QPushButton("Paste XML", &dialog);
  QPushButton *cancelButton = new QPushButton("Cancel", &dialog);
  
  layout->addWidget(fileButton);
  layout->addWidget(pasteButton);
  layout->addWidget(cancelButton);
  
  connect(fileButton, &QPushButton::clicked, &dialog, &QDialog::accept);
  connect(pasteButton, &QPushButton::clicked, &dialog, &QDialog::accept);
  connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
  
  // Store which button was clicked
  QObject::connect(fileButton, &QPushButton::clicked, [&dialog]() {
    dialog.setProperty("method", "file");
  });
  QObject::connect(pasteButton, &QPushButton::clicked, [&dialog]() {
    dialog.setProperty("method", "paste");
  });
  
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  
  QString method = dialog.property("method").toString();
  QString xmlContent;
  
  if (method == "file") {
    QString fileName = QFileDialog::getOpenFileName(this, "Import Burp XML", "", "XML Files (*.xml)");
    if (fileName.isEmpty()) {
      return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QMessageBox::warning(this, "Import Error", "Could not open file: " + fileName);
      return;
    }
    
    QTextStream in(&file);
    xmlContent = in.readAll();
    file.close();
  } else if (method == "paste") {
    // Dialog with textarea for pasting XML
    QDialog pasteDialog(this);
    pasteDialog.setWindowTitle("Paste Burp XML");
    pasteDialog.setMinimumSize(600, 400);
    
    QVBoxLayout *pasteLayout = new QVBoxLayout(&pasteDialog);
    
    QLabel *pasteLabel = new QLabel("Paste XML content:", &pasteDialog);
    pasteLayout->addWidget(pasteLabel);
    
    QTextEdit *textEdit = new QTextEdit(&pasteDialog);
    textEdit->setFont(QFont("monospace"));
    pasteLayout->addWidget(textEdit);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &pasteDialog);
    pasteLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &pasteDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &pasteDialog, &QDialog::reject);
    
    if (pasteDialog.exec() != QDialog::Accepted) {
      return;
    }
    
    xmlContent = textEdit->toPlainText();
    if (xmlContent.isEmpty()) {
      QMessageBox::warning(this, "Import Error", "No XML content provided.");
      return;
    }
  } else {
    return;
  }
  
  // Parse XML from string
  QXmlStreamReader xml(xmlContent);
  QModelIndex parentIndex = getSelectedIndex();
  
  int importedCount = 0;
  int errorCount = 0;

  while (!xml.atEnd() && !xml.hasError()) {
    QXmlStreamReader::TokenType token = xml.readNext();
    
    if (token == QXmlStreamReader::StartElement && xml.name() == QLatin1String("item")) {
      QString timeStr, urlStr, hostStr, portStr, protocolStr, methodStr, pathStr;
      QString requestBase64, responseBase64, commentStr;
      int status = 0;
      qint64 responseLength = 0;
      QString mimeType;

      while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QLatin1String("item"))) {
        xml.readNext();
        
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
          QString elementName = xml.name().toString();
          
          if (elementName == "time") {
            timeStr = xml.readElementText();
          } else if (elementName == "url") {
            urlStr = xml.readElementText();
          } else if (elementName == "host") {
            hostStr = xml.readElementText();
          } else if (elementName == "port") {
            portStr = xml.readElementText();
          } else if (elementName == "protocol") {
            protocolStr = xml.readElementText();
          } else if (elementName == "method") {
            methodStr = xml.readElementText();
          } else if (elementName == "path") {
            pathStr = xml.readElementText();
          } else if (elementName == "request") {
            QString base64Attr = xml.attributes().value("base64").toString();
            if (base64Attr == "true") {
              requestBase64 = xml.readElementText();
            } else {
              // If not base64, encode it
              QString text = xml.readElementText();
              requestBase64 = text.toUtf8().toBase64();
            }
          } else if (elementName == "status") {
            status = xml.readElementText().toInt();
          } else if (elementName == "responselength") {
            responseLength = xml.readElementText().toLongLong();
          } else if (elementName == "mimetype") {
            mimeType = xml.readElementText();
          } else if (elementName == "response") {
            QString base64Attr = xml.attributes().value("base64").toString();
            if (base64Attr == "true") {
              responseBase64 = xml.readElementText();
            } else {
              // If not base64, encode it
              QString text = xml.readElementText();
              responseBase64 = text.toUtf8().toBase64();
            }
          } else if (elementName == "comment") {
            commentStr = xml.readElementText();
          }
        }
      }

      // Create request item
      if (!methodStr.isEmpty() && !urlStr.isEmpty()) {
        // Generate name from URL and method
        QUrl url(urlStr);
        QString path = url.path();
        if (path.isEmpty()) {
          path = "/";
        }
        QString name = methodStr + " " + path;
        if (name.length() > 50) {
          name = name.left(47) + "...";
        }
        
        // Extract query string from URL
        QString queryStr = url.query();
        
        // Use pathStr if available, otherwise extract from URL
        QString pathToUse = pathStr.isEmpty() ? path : pathStr;

        QModelIndex newIndex = m_model->addRequest(name, parentIndex);
        OrganizerItem* item = m_model->getItem(newIndex);
        
        if (item) {
          // Set basic fields
          item->setHost(hostStr);
          item->setUrl(pathToUse);  // URL field contains only the path
          item->setMethod(methodStr);
          item->setQuery(queryStr);
          item->setStatus(status);
          item->setLength(responseLength);
          item->setAnnotation(commentStr);
          
          // Set request and response (already base64)
          if (!requestBase64.isEmpty()) {
            item->setRequest(requestBase64);
          }
          if (!responseBase64.isEmpty()) {
            item->setResponse(responseBase64);
          }
          
          // Parse timestamp
          if (!timeStr.isEmpty()) {
            QDateTime dateTime;
            // Try different date formats
            QStringList formats = {
              "ddd MMM dd hh:mm:ss 'CST' yyyy",
              "ddd MMM dd hh:mm:ss 'EST' yyyy",
              "ddd MMM dd hh:mm:ss 'PST' yyyy",
              "ddd MMM dd hh:mm:ss 'GMT' yyyy",
              "ddd MMM dd hh:mm:ss yyyy"
            };
            
            for (const QString& format : formats) {
              dateTime = QDateTime::fromString(timeStr, format);
              if (dateTime.isValid()) {
                break;
              }
            }
            
            // Try ISO format if other formats failed
            if (!dateTime.isValid()) {
              dateTime = QDateTime::fromString(timeStr, Qt::ISODate);
            }
            
            if (dateTime.isValid()) {
              item->setTimestamp(dateTime.toSecsSinceEpoch());
            } else {
              // If parsing fails, use current time
              item->setTimestamp(QDateTime::currentDateTime().toSecsSinceEpoch());
            }
          } else {
            // If no time, use current time
            item->setTimestamp(QDateTime::currentDateTime().toSecsSinceEpoch());
          }
          
          // Save to database
          m_model->saveItem(newIndex);
          importedCount++;
        } else {
          errorCount++;
        }
      } else {
        errorCount++;
      }
    }
  }

  if (xml.hasError()) {
    QMessageBox::warning(this, "Import Error", "XML parsing error: " + xml.errorString());
    return;
  }

  // Expand parent if valid
  if (parentIndex.isValid()) {
    m_treeView->expand(parentIndex);
  }

  QMessageBox::information(this, "Import Complete", 
    QString("Imported %1 requests successfully.\n%2 errors occurred.")
    .arg(importedCount).arg(errorCount));
}
