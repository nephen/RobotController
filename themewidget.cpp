#include "themewidget.h"
#include "ui_themewidget.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QComboBox>
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
    speedTargetSplineSeries(new QSplineSeries),
    positionSplineSeries(new QSplineSeries),
    positionTargetSplineSeries(new QSplineSeries),
    currentSplineSeries(new QSplineSeries),
    currentTargetSplineSeries(new QSplineSeries),
    m_speed_axis(new QValueAxis),
    m_position_axis(new QValueAxis),
    m_current_axis(new QValueAxis),
    speedTargetLayout(new QHBoxLayout),
    positionTargetLayout(new QHBoxLayout),
    currentTargetLayout(new QHBoxLayout)
{
    m_ui->setupUi(this);

    initValues();
    populateThemeBox();
    populateAnimationBox();
    populateLegendBox();

    createTargetLayout();

    createChartsLayout();

    setAxis();

    // Set defaults
    m_ui->antialiasCheckBox->setChecked(true);
    m_ui->animatedComboBox->setCurrentIndex(2);
    m_ui->themeComboBox->setCurrentIndex(2);

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

void ThemeWidget::createChartsLayout()
{
    //create charts
    ChartView *chartView;

    speedSplineChart = createSpeedSplineChart();
    chartView = new ChartView(speedSplineChart);
    m_ui->chartsGridLayout->addWidget(chartView, 1, 0);
    m_charts << chartView;

    positionSplineChart = createPositionSplineChart();
    chartView = new ChartView(positionSplineChart);
    m_ui->chartsGridLayout->addWidget(chartView, 1, 1);
    m_charts << chartView;

    m_ui->chartsGridLayout->addLayout(speedTargetLayout, 2, 0);
    m_ui->chartsGridLayout->addLayout(positionTargetLayout, 2, 1);

    currentSplineChart = createCurrentSplineChart();
    chartView = new ChartView(currentSplineChart);
    m_ui->chartsGridLayout->addWidget(chartView, 3, 0, 1, 2);
    m_charts << chartView;

    m_ui->chartsGridLayout->addLayout(currentTargetLayout, 4, 0, 1, 2);
}

void ThemeWidget::createTargetLayout()
{
    speedTargetValueDoubleSpinBox = new QDoubleSpinBox();
    QLabel *speedTargetValueLabel = new QLabel(tr("speed target:"));

    positionTargetValueDoubleSpinBox = new QDoubleSpinBox();
    QLabel *positionTargetValueLabel = new QLabel(tr("position target:"));

    currentTargetValueDoubleSpinBox = new QDoubleSpinBox();
    QLabel *currentTargetValueLabel = new QLabel(tr("current target:"));

    speedTargetLayout->addWidget(speedTargetValueLabel);
    speedTargetLayout->addWidget(speedTargetValueDoubleSpinBox);
    speedTargetLayout->addStretch(20);
    positionTargetLayout->addWidget(positionTargetValueLabel);
    positionTargetLayout->addWidget(positionTargetValueDoubleSpinBox);
    positionTargetLayout->addStretch(20);
    currentTargetLayout->addWidget(currentTargetValueLabel);
    currentTargetLayout->addWidget(currentTargetValueDoubleSpinBox);
    currentTargetLayout->addStretch(20);
}

void ThemeWidget::initValues()
{
    //Vertical coordinate maximum
    m_speedValueMax = 8;
    m_positionValueMax = 20;
    m_currentValueMax = 40;

    //Horizontal coordinate maximum
    m_speedValueCount = 100;
    m_positionValueCount = 100;
    m_currentValueCount = 100;

    //The initial coordinate is displayed in the horizontal coordinate
    m_speed_x = m_speedValueCount;
    m_position_x = m_positionValueCount;
    m_current_x = m_currentValueCount;

    //Raw data received from the motor controller
    m_received_value_y = 0;
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
    chart->addSeries(speedTargetSplineSeries);

    chart->createDefaultAxes();
    chart->axisX()->setRange(0, m_speedValueCount);
    chart->axisY()->setRange(0, m_speedValueMax);
    chart->axisX()->setTitleText("Time:s");
    chart->axisY()->setTitleText("Speed:RPM");
//    QValueAxis * test = qobject_cast<QValueAxis *>(speedTargetSplineSeries->attachedAxes());
//    qDebug() << test->max();
    // Add space to label to add space between labels and axis
    static_cast<QValueAxis *>(chart->axisY())->setLabelFormat("%.1f  ");

    return chart;
}

QChart *ThemeWidget::createPositionSplineChart() const
{
    QChart *chart = new QChart();
    chart->setTitle("position spline chart");
    chart->addSeries(positionSplineSeries);
    chart->addSeries(positionTargetSplineSeries);

    chart->createDefaultAxes();
    chart->axisX()->setRange(0, m_positionValueCount);
    chart->axisY()->setRange(0, m_positionValueMax);
    chart->axisX()->setTitleText("Time:s");
    chart->axisY()->setTitleText("Position:°");
    // Add space to label to add space between labels and axis
    static_cast<QValueAxis *>(chart->axisY())->setLabelFormat("%.1f  ");

    return chart;
}

QChart *ThemeWidget::createCurrentSplineChart() const
{
    QChart *chart = new QChart();
    chart->setTitle("current spline chart");
    chart->addSeries(currentSplineSeries);
    chart->addSeries(currentTargetSplineSeries);

    chart->createDefaultAxes();
    chart->axisX()->setRange(0, m_currentValueCount);
    chart->axisY()->setRange(0, m_currentValueMax);
    chart->axisX()->setTitleText("Time:s");
    chart->axisY()->setTitleText("Current:A");
    // Add space to label to add space between labels and axis
    static_cast<QValueAxis *>(chart->axisY())->setLabelFormat("%.1f  ");

    return chart;
}

//Set the number of subdivisions of the axis
void ThemeWidget::setAxis(void)
{
    m_speed_axis->setTickCount(10);
    m_position_axis->setTickCount(5);
    m_current_axis->setTickCount(20);
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
        for (ChartView *chartView : charts) {
            //![7]
            chartView->chart()->setTheme(theme);
            //![7]
        }

        // Set palette colors based on selected theme
        //![8]
        QPalette pal = palette(); //If you want to change the color of the entire window, use window()->palette()
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
        setPalette(pal);
    }

    // Update antialiasing
    //![11]
    bool checked = m_ui->antialiasCheckBox->isChecked();
    for (ChartView *chart : charts)
        chart->setRenderHint(QPainter::Antialiasing, checked);
    //![11]

    // Update animation options
    //![9]
    QChart::AnimationOptions options(
                m_ui->animatedComboBox->itemData(m_ui->animatedComboBox->currentIndex()).toInt());
    if (!m_charts.isEmpty() && m_charts.at(0)->chart()->animationOptions() != options) {
        for (ChartView *chartView : charts)
            chartView->chart()->setAnimationOptions(options);
    }
    //![9]

    // Update legend alignment
    //![10]
    Qt::Alignment alignment(
                m_ui->legendComboBox->itemData(m_ui->legendComboBox->currentIndex()).toInt());

    if (!alignment) {
        for (ChartView *chartView : charts)
            chartView->chart()->legend()->hide();
    } else {
        for (ChartView *chartView : charts) {
            chartView->chart()->legend()->setAlignment(alignment);
            chartView->chart()->legend()->show();
        }
    }
    //![10]
}

void ThemeWidget::handleTimeout()
{
    qreal x,y;

    x = speedSplineChart->plotArea().width()/m_speed_axis->tickCount();
    y = m_speedValueCount/m_speed_axis->tickCount();
//    qDebug() << m_speed_axis->max() << m_speed_axis->min();
    m_speed_x += y;
    m_received_value_y = QRandomGenerator::global()->bounded(m_speedValueMax);   //received speed value
    speedSplineSeries->append(m_speed_x, m_received_value_y);
    speedTargetSplineSeries->append(m_speed_x, speedTargetValueDoubleSpinBox->value());
    speedSplineChart->scroll(x, 0);

    x = positionSplineChart->plotArea().width()/m_position_axis->tickCount();
    y = m_positionValueCount/m_position_axis->tickCount();
    m_position_x += y;
    m_received_value_y = QRandomGenerator::global()->bounded(m_positionValueMax);   //received position value
    positionSplineSeries->append(m_position_x, m_received_value_y);
    positionTargetSplineSeries->append(m_position_x, positionTargetValueDoubleSpinBox->value());
    positionSplineChart->scroll(x, 0);

    x = currentSplineChart->plotArea().width()/m_current_axis->tickCount();
    y = m_currentValueCount/m_current_axis->tickCount();
    m_current_x += y;
    m_received_value_y = QRandomGenerator::global()->bounded(m_currentValueMax);   //received current value
    currentSplineSeries->append(m_current_x, m_received_value_y);
    currentTargetSplineSeries->append(m_current_x, currentTargetValueDoubleSpinBox->value());
    currentSplineChart->scroll(x, 0);
}
