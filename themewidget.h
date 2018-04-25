#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include <QGroupBox>
#include <QtWidgets/QWidget>
#include <QtCharts/QChartGlobal>
#include <QTimer>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QDoubleSpinBox>
#include "chartview.h"

namespace Ui {
class ThemeWidget;
}

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
QT_END_NAMESPACE

QT_CHARTS_BEGIN_NAMESPACE
class QChart;
QT_CHARTS_END_NAMESPACE

typedef QPair<QPointF, QString> Data;
typedef QList<Data> DataList;
typedef QList<DataList> DataTable;

QT_CHARTS_USE_NAMESPACE

class ThemeWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit ThemeWidget(QWidget *parent = 0);
    ~ThemeWidget();

private slots:
    void updateUI();

public slots:
    void handleTimeout();

private:
    void populateThemeBox();
    void populateAnimationBox();
    void populateLegendBox();
    void initBoxConnections();
    void initChartTimer();
    void initValues();
    void setAxis();
    void createTargetLayout();
    void createChartsLayout();
    QChart *createSpeedSplineChart() const;
    QChart *createPositionSplineChart() const;
    QChart *createCurrentSplineChart() const;

private:
    int m_speedValueMax;
    int m_positionValueMax;
    int m_currentValueMax;

    int m_speedValueCount;
    int m_positionValueCount;
    int m_currentValueCount;

    QList<ChartView *> m_charts;
    Ui::ThemeWidget *m_ui;
    QTimer m_timer;

    QChart *speedSplineChart;
    QChart *positionSplineChart;
    QChart *currentSplineChart;

    //Including the raw data sent by the motor controller and the current target value
    QSplineSeries *speedSplineSeries;
    QSplineSeries *speedTargetSplineSeries;
    QSplineSeries *positionSplineSeries;
    QSplineSeries *positionTargetSplineSeries;
    QSplineSeries *currentSplineSeries;
    QSplineSeries *currentTargetSplineSeries;
    QValueAxis *m_speed_axis;
    QValueAxis *m_position_axis;
    QValueAxis *m_current_axis;

    qreal m_speed_x;
    qreal m_position_x;
    qreal m_current_x;
    qreal m_received_value_y;

    QHBoxLayout * speedTargetLayout;
    QHBoxLayout * positionTargetLayout;
    QHBoxLayout * currentTargetLayout;

    QDoubleSpinBox * speedTargetValueDoubleSpinBox;
    QDoubleSpinBox * positionTargetValueDoubleSpinBox;
    QDoubleSpinBox * currentTargetValueDoubleSpinBox;
};

#endif // THEMEWIDGET_H
