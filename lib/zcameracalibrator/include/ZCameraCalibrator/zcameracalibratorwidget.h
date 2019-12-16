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

#include <Z3DCalibratedCamera>
#include <Z3DCameraCalibration>

#include "ZGui/zwidget.h"

#include <QModelIndex>

class QProgressBar;
class QThread;

class ObjectController;

namespace Z3D
{

namespace Ui {
class ZCameraCalibratorWidget;
}

class Z3D_CAMERACALIBRATOR_SHARED_EXPORT ZCameraCalibratorWidget : public ZWidget
{
    Q_OBJECT

public:
    explicit ZCameraCalibratorWidget(Z3D::ZCalibratedCameraPtr camera = nullptr, QWidget *parent = nullptr);
    ~ZCameraCalibratorWidget();

private slots:
    virtual void closeEvent(QCloseEvent *event);

    void updateWindowTitle();

    void setCurrentView(int newView);

    void setImagesListVisible(bool visible);

    void onCurrentSelectionChanged(QModelIndex current, QModelIndex previous);

    void onCameraModelTypeChanged(int index);
    void onCalibrationPatternTypeChanged(int index);

    void onProgressChanged(float progress, QString message);
    void onCalibrationChanged(Z3D::ZCameraCalibrationPtr newCalibration);

    void loadSession();
    void newSession();

    void on_calibrationPatternViewButton_clicked();
    void on_imageViewPageButton_clicked();
    void on_cameraViewPageButton_clicked();
    void on_calibrationPageButton_clicked();
    void on_resultsPageButton_clicked();

    void on_runCalibrationButton_clicked();

    void on_imageViewTypeComboBox_currentIndexChanged(int index);

    void on_saveCameraImageButton_clicked();

    void on_cameraSettingsButton_clicked();

    void on_saveCalibrationToFileButton_clicked();

    void on_updateCameraCalibrationButton_clicked();

private:
    Ui::ZCameraCalibratorWidget *ui;
    QProgressBar *m_statusProgressBar;

    ZCalibrationImageModel *m_model;
    QString m_sessionFolder;

    QList<ZCalibrationPatternFinderPtr> m_patternFinderList;
    QList<ZCameraCalibrator *> m_cameraCalibratorList;

    ZCameraCalibratorWorker *m_calibratorWorker;

    Z3D::ZCalibratedCameraPtr m_camera;

    Z3D::ZCameraCalibrationPtr m_currentCalibration;

    ObjectController *m_calibrationParamsController;

    QThread *m_workerThread;
};

} // namespace Z3D
