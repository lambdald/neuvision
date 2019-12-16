//
// Z3D - A structured light 3D scanner
// Copyright (C) 2013-2016 Nicolas Ulrich <nikolaseu@gmail.com>
//
// This file is part of Z3D.
//
// Z3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Z3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Z3D.  If not, see <http://www.gnu.org/licenses/>.
//

#include "ZCameraCalibrator/zcalibrationimagemodel.h"

#include "ZCameraCalibrator/zcalibrationimage.h"

#include <QDebug>
#include <QDir>
#include <QtConcurrent>

namespace Z3D
{

ZCalibrationImageModel::ZCalibrationImageModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_width(0)
    , m_height(0)
{
    /// to notify when the loading of images has finished
    QObject::connect(&m_futureWatcher, &QFutureWatcher<void>::finished,
                     this, &ZCalibrationImageModel::newImagesAdded);
}

QHash<int, QByteArray> ZCalibrationImageModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DataRole] = "calibrationImage";
    return roles;
}

int ZCalibrationImageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return m_images.size();
}

QVariant ZCalibrationImageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_images.size())
        return QVariant();

    Z3D::ZCalibrationImagePtr imageObject = imageAt(index.row());

    switch (role) {
    case DataRole:
        return QVariant::fromValue(imageObject);
    case Qt::ToolTipRole:
    case FilenameRole:
        return imageObject->fileName();
    case Qt::DecorationRole:
        return imageObject->imageFromUrl(Z3D::ZCalibrationImage::ThumbnailImage);
    }

    return QVariant();
}

const QList<ZCalibrationImagePtr> &ZCalibrationImageModel::images() const
{
    return m_images;
}

void ZCalibrationImageModel::clear()
{
    beginResetModel();

    //! clear list
    m_images.clear();

    endResetModel();
}

void ZCalibrationImageModel::addFolder(QString folder)
{
    /// we must wait for the previous task to finish
    /// because we use the static vector "images"
    if (m_futureWatcher.isRunning()) {
        qWarning() << "waiting for previous task to finish...";
        m_futureWatcher.waitForFinished();
    }

    qDebug() << "loading calibration images from" << folder;

    QStringList filters;
    filters << "*.jpg" << "*.bmp" << "*.png";

    QDir imageDir(folder);

    QStringList imageFiles = imageDir.entryList(filters, QDir::Files, QDir::Name);

    /// this is the part that we need to avoid running when the concurrent
    /// loading still hasn't finished
    m_imagesToLoad.clear();
    m_imagesToLoad.reserve(imageFiles.size());

    /// images must be created from this thread, this is required by the Qml engine
    for (const auto &file : imageFiles) {
        Z3D::ZCalibrationImagePtr image( new ZCalibrationImage(imageDir.absoluteFilePath(file)) );
        m_imagesToLoad.push_back(image);
    }

    /// add images to model
    addImages(m_imagesToLoad);
}

void ZCalibrationImageModel::addImage(ZCalibrationImagePtr image)
{
    /// we must wait for the previous task to finish
    /// because we use the static vector "images"
    if (m_futureWatcher.isRunning()) {
        qWarning() << "waiting for previous task to finish...";
        m_futureWatcher.waitForFinished();
    }

    m_imagesToLoad.clear();
    m_imagesToLoad.push_back(image);

    /// add images to model
    addImages(m_imagesToLoad);
}

void ZCalibrationImageModel::addImageThreadSafe(ZCalibrationImagePtr image)
{
    /// executes addImpl from the object thread, not from the thread where this
    /// function is called. this is needed for the view to update correctly when
    /// we add images in parallel (like in addImages, using QtConcurrent::map)
    QMetaObject::invokeMethod(this, "addImpl", Qt::AutoConnection,
                              Q_ARG(Z3D::ZCalibrationImagePtr, image));
}

void ZCalibrationImageModel::addImages(const QVector<Z3D::ZCalibrationImagePtr > &images)
{
    /// parallelize the checking of image validity before adding to model
    m_futureWatcher.setFuture(QtConcurrent::map(images, [=](const auto &image) {
        if (image->isValid()) {
            this->addImageThreadSafe(image);
        }
    }));
}

void ZCalibrationImageModel::addImpl(ZCalibrationImagePtr image)
{
    /// to avoid entering this section from more than one thread at a time
    QMutexLocker mutexLocker(&m_mutex);

    int row = m_images.size();

    if (row < 1) {
        m_width = image->width();
        m_height = image->height();
        emit imageSizeChanged();
        qDebug() << "image size:" << m_width << "x" << m_height;
    }

    if (image->width() != m_width || image->height() != m_height) {
        qWarning() << "image not added. it is not the same size:" << image->fileName();
        return;
    }

    beginInsertRows(QModelIndex(), row, row);

    //! add to list
    m_images.insert(row, image);

    endInsertRows();

    //! connect to image signals
    QObject::connect(image.get(), &ZCalibrationImage::stateChanged,
                     this, &ZCalibrationImageModel::onImageStateChanged);
}

Z3D::ZCalibrationImagePtr ZCalibrationImageModel::imageAt(int index) const
{
    if (index >= 0 && index < m_images.size()) {
        return m_images[index];
    } else {
        qCritical() << "invalid image index requested:" << index << " - model size:" << m_images.size();
        return nullptr;
    }
}

void ZCalibrationImageModel::onImageStateChanged()
{
    Z3D::ZCalibrationImage *changedImage = qobject_cast<Z3D::ZCalibrationImage*>(sender());
    for (int i=0; i<m_images.size(); ++i) {
        if (m_images[i].get() == changedImage) {
            emit dataChanged(index(i), index(i));
            break;
        }
    }
}

} // namespace Z3D
