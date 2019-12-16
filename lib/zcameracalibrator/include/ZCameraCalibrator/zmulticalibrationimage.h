/* * Z3D - A structured light 3D scanner
 * Copyright (C) 2013-2016 Nicolas Ulrich <nikolaseu@gmail.com>
 *
 * This file is part of Z3D.
 *
 * Z3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Z3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Z3D.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "ZCameraCalibrator/zcameracalibrator_fwd.h"
#include "ZCameraCalibrator/zcameracalibrator_global.h"

#include "ZCameraCalibrator/zcalibrationimage.h" // required here for Z3D::ZCalibrationImage::ImageState

#include <QObject>

namespace Z3D
{

class Z3D_CAMERACALIBRATOR_SHARED_EXPORT ZMultiCalibrationImage : public QObject
{
    Q_OBJECT

public:
    ZMultiCalibrationImage(QList<Z3D::ZCalibrationImagePtr> images);
    ~ZMultiCalibrationImage();

    bool isValid();
    QString fileName() const;

    /// image thumbnail
    QImage thumbnail() const;

    const QList<Z3D::ZCalibrationImagePtr> &images() const;

    Z3D::ZCalibrationImagePtr image(int index) const;

    Z3D::ZCalibrationImage::ImageState state() const;

signals:
    void stateChanged(Z3D::ZCalibrationImage::ImageState arg);

public slots:

protected slots:
    void updateState();

protected:
    void setState(Z3D::ZCalibrationImage::ImageState arg);

    QList<Z3D::ZCalibrationImagePtr> m_images;

    Z3D::ZCalibrationImage::ImageState m_state;
};

} // namespace Z3D

Q_DECLARE_METATYPE(Z3D::ZMultiCalibrationImagePtr) /// to use in QVariant
