#include "themewidget.h"
#include "ui_themewidget.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtCore/QRandomGenerator>
#include <QtWidgets/QApplication>
#include <QtCharts/QValueAxis>
#include <QDebug>

ThemeWidget::ThemeWidget(QWidget *parent) :
    QGroupBox(parent),
    m_ui(new Ui::ThemeWidget),
    speedSplineChart(new QChart),
    positionSplineChart(new QChart),
    currentSplineChart(new QChart),
    speedSplineSeries(new QSplineSeries),
    positionSplineSeries(new QSplineSeries),
    currentSplineSeries(new QSplineSeries),
    m_speed_axis(new QValueAxis),
    m_position_axis(new QValueAxis),
    m_current_axis(new QValueAxis)
{
    m_ui->setupUi(this);

    initValues();
    populateThemeBox();
    populateAnimationBox();
    populateLegendBox();

    //create charts

    QChartView *chartView;

    speedSplineChart = createSpeedSplineChart();
    chartView = new QChartView(speedSplineChart);
    m_ui->chartsGridLayout->addWidget(chartView, 1, 0);
    m_charts << chartView;

    positionSplineChart = createPositionSplineChart();
    chartView = new QChartView(positionSplineChart);
    m_ui->chartsGridLayout->addWidget(chartView, 1, 1);
    m_charts << chartView;

    currentSplineChart = createCurrentSplineChart();
    chartView = new QChartView(currentSplineChart);
    m_ui->chartsGridLayout->addWidget(chartView, 2, 0);
    m_charts << chartView;

//    currentSplineChart = createCurrentSplineChart();
//    chartView = new QChartView(currentSplineChart);
//    m_ui->chartsGridLayout->addWidget(chartView, 2, 1);
//    m_charts << chartView;
    // Set defaults
    m_ui->antialiasCheckBox->setChecked(true);

    // Set the colors from the light theme as default ones
    QPalette pal = qApp->palette();
    pal.setColor(QPalette::Window, QRgb(0xf0f0f0));
    pal.setColor(QPalette::WindowText, QRgb(0x404044));
    qApp->setPalette(pal);

    initBoxConnections();

    initChartTimer();

    updateUI();
}

ThemeWidget::~ThemeWidget()
{
    delete m_ui;
}

void ThemeWidget::initValues()
{
    m_speedValueMax = 8;
    m_positionValueMax = 8;
    m_currentValueMax = 8;

    m_speedValueCount = 100;
    m_positionValueCount = 100;
    m_currentValueCount = 100;

    m_x = 100;
    m_y = 8;
}

void ThemeWidget::initChartTimer()
{
    QObject::connect(&m_timer, &QTimer::timeout, this, &ThemeWidget::handleTimeout);
    m_timer.setInterval(300);
    m_timer.start();
}

void ThemeWidget::initBoxConnections()
{
    QObject::connect(m_ui->themeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()));
    QObject::connect(m_ui->antialiasCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateUI()));
    QObject::connect(m_ui->legendComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()));
    QObject::connect(m_ui->animatedComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()));
}

void ThemeWidget::populateThemeBox()
{
    // add items to theme combobox
    m_ui->themeComboBox->addItem("Light", QChart::ChartThemeLight);
    m_ui->themeComboBox->addItem("Blue Cerulean", QChart::ChartThemeBlueCerulean);
    m_ui->themeComboBox->addItem("Dark", QChart::ChartThemeDark);
    m_ui->themeComboBox->addItem("Brown Sand", QChart::ChartThemeBrownSand);
    m_ui->themeComboBox->addItem("Blue NCS", QChart::ChartThemeBlueNcs);
    m_ui->themeComboBox->addItem("High Contrast", QChart::ChartThemeHighContrast);
    m_ui->themeComboBox->addItem("Blue Icy", QChart::ChartThemeBlueIcy);
    m_ui->themeComboBox->addItem("Qt", QChart::ChartThemeQt);
}

void ThemeWidget::populateAnimationBox()
{
    // add items to animation combobox
    m_ui->animatedComboBox->addItem("No Animations", QChart::NoAnimation);
    m_ui->animatedComboBox->addItem("GridAxis Animations", QChart::GridAxisAnimations);
    m_ui->animatedComboBox->addItem("Series Animations", QChart::SeriesAnimations);
    m_ui->animatedComboBox->addItem("All Animations", QChart::AllAnimations);
}

void ThemeWidget::populateLegendBox()
{
    // add items to legend combobox
    m_ui->legendComboBox->addItem("No Legend ", 0);
    m_ui->legendComboBox->addItem("Legend Top", Qt::AlignTop);
    m_ui->legendComboBox->addItem("Legend Bottom", Qt::AlignBottom);
    m_ui->legendComboBox->addItem("Legend Left", Qt::AlignLeft);
    m_ui->legendComboBox->addItem("Legend Right", Qt::AlignRight);
}

QChart *ThemeWidget::createSpeedSplineChart() const
{
    QChart *chart = new QChart();
    chart->setTitle("speed spline chart");
    chart->addSeries(speedSplineSeries);

    m_speed_axis->setTickCount(5);

    chart->createDefaultAxes();
    chart->setAxisX(m_speed_axis,speedSplineSeries);
    chart->axisX()->setRange(0, m_speedValueCount);
    chart->axisY()->setRange(0, m_speedValueMax);
    // Add space to label to add space between labels and axis
    static_cast<QValueAxis *>(chart->axisY())->setLabelFormat("%.1f  ");

    return chart;
}

QChart *ThemeWidget::createPositionSplineChart() const
{
    QChart *chart = new QChart();
    chart->setTitle("position spline chart");
    chart->addSeries(positionSplineSeries);

    m_position_axis->setTickCount(5);

    chart->createDefaultAxes();
    chart->setAxisX(m_position_axis,positionSplineSeries);
    chart->axisX()->setRange(0, m_positionValueCount);
    chart->axisY()->setRange(0, m_positionValueMax);
    // Add space to label to add space between labels and axis
    static_cast<QValueAxis *>(chart->axisY())->setLabelFormat("%.1f  ");

    return chart;
}

QChart *ThemeWidget::createCurrentSplineChart() const
{
    QChart *chart = new QChart();
    chart->setTitle("current spline chart");
    chart->addSeries(currentSplineSeries);

    m_current_axis->setTickCount(20);

    chart->createDefaultAxes();
    chart->setAxisX(m_current_axis,currentSplineSeries);
    chart->axisX()->setRange(0, m_currentValueCount);
    chart->axisY()->setRange(0, m_currentValueMax);
    // Add space to label to add space between labels and axis
    static_cast<QValueAxis *>(chart->axisY())->setLabelFormat("%.1f  ");

    return chart;
}

void ThemeWidget::updateUI()
{
    qDebug() << "updateUI";
    //![6]
    QChart::ChartTheme theme = static_cast<QChart::ChartTheme>(
                m_ui->themeComboBox->itemData(m_ui->themeComboBox->currentIndex()).toInt());
    //![6]
    const auto charts = m_charts;
    if (!m_charts.isEmpty() && m_charts.at(0)->chart()->theme() != theme) {
        for (QChartView *chartView : charts) {
            //![7]
            chartView->chart()->setTheme(theme);
            //![7]
        }

        // Set palette colors based on selected theme
        //![8]
        QPalette pal = window()->palette();
        if (theme == QChart::ChartThemeLight) {
            pal.setColor(QPalette::Window, QRgb(0xf0f0f0));
            pal.setColor(QPalette::WindowText, QRgb(0x404044));
        //![8]
        } else if (theme == QChart::ChartThemeDark) {
            pal.setColor(QPalette::Window, QRgb(0x121218));
            pal.setColor(QPalette::WindowText, QRgb(0xd6d6d6));
        } else if (theme == QChart::ChartThemeBlueCerulean) {
            pal.setColor(QPalette::Window, QRgb(0x40434a));
            pal.setColor(QPalette::WindowText, QRgb(0xd6d6d6));
        } else if (theme == QChart::ChartThemeBrownSand) {
            pal.setColor(QPalette::Window, QRgb(0x9e8965));
            pal.setColor(QPalette::WindowText, QRgb(0x404044));
        } else if (theme == QChart::ChartThemeBlueNcs) {
            pal.setColor(QPalette::Window, QRgb(0x018bba));
            pal.setColor(QPalette::WindowText, QRgb(0x404044));
        } else if (theme == QChart::ChartThemeHighContrast) {
            pal.setColor(QPalette::Window, QRgb(0xffab03));
            pal.setColor(QPalette::WindowText, QRgb(0x181818));
        } else if (theme == QChart::ChartThemeBlueIcy) {
            pal.setColor(QPalette::Window, QRgb(0xcee7f0));
            pal.setColor(QPalette::WindowText, QRgb(0x404044));
        } else {
            pal.setColor(QPalette::Window, QRgb(0xf0f0f0));
            pal.setColor(QPalette::WindowText, QRgb(0x404044));
        }
        window()->setPalette(pal);
    }

    // Update antialiasing
    //![11]
    bool checked = m_ui->antialiasCheckBox->isChecked();
    for (QChartView *chart : charts)
        chart->setRenderHint(QPainter::Antialiasing, checked);
    //![11]

    // Update animation options
    //![9]
    QChart::AnimationOptions options(
                m_ui->animatedComboBox->itemData(m_ui->animatedComboBox->currentIndex()).toInt());
    if (!m_charts.isEmpty() && m_charts.at(0)->chart()->animationOptions() != options) {
        for (QChartView *chartView : charts)
            chartView->chart()->setAnimationOptions(options);
    }
    //![9]

    // Update legend alignment
    //![10]
    Qt::Alignment alignment(
                m_ui->legendComboBox->itemData(m_ui->legendComboBox->currentIndex()).toInt());

    if (!alignment) {
        for (QChartView *chartView : charts)
            chartView->chart()->legend()->hide();
    } else {
        for (QChartView *chartView : charts) {
            chartView->chart()->legend()->setAlignment(alignment);
            chartView->chart()->legend()->show();
        }
    }
    //![10]
}

void ThemeWidget::handleTimeout()
{
    qDebug() << "updateChars";

    qreal x = speedSplineChart->plotArea().width()/m_speed_axis->tickCount();
    qreal y = (m_speed_axis->max() - m_speed_axis->min())/m_speed_axis->tickCount();

    qDebug() << m_speed_axis->max() << m_speed_axis->min();
    m_x += y;
    m_y = QRandomGenerator::global()->bounded(8);
    qDebug() << m_x << m_y;
    speedSplineSeries->append(m_x, m_y);
    speedSplineChart->scroll(x, 0);

    positionSplineSeries->append(m_x, m_y);
    positionSplineChart->scroll(x, 0);

    currentSplineSeries->append(m_x, m_y);
    currentSplineChart->scroll(x, 0);
}
