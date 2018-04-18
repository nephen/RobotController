#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include <QGroupBox>
#include <QtWidgets/QWidget>
#include <QtCharts/QChartGlobal>
#include <QTimer>
#include <QtCharts/QSplineSeries>

namespace Ui {
class ThemeWidget;
}

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
QT_END_NAMESPACE

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
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
    DataTable generateRandomData(int listCount, int valueMax, int valueCount) const;
    void populateThemeBox();
    void populateAnimationBox();
    void populateLegendBox();
    void initBoxConnections();
    void initChartTimer();
    QChart *createAreaChart() const;
    QChart *createBarChart(int valueCount) const;
    QChart *createPieChart() const;
    QChart *createLineChart() const;
    QChart *createSplineChart() const;
    QChart *createScatterChart() const;

private:
    int m_listCount;
    int m_valueMax;
    int m_valueCount;
    QList<QChartView *> m_charts;
    DataTable m_dataTable;
    Ui::ThemeWidget *m_ui;
    QTimer m_timer;
    QSplineSeries *splineSeries;
};

#endif // THEMEWIDGET_H
