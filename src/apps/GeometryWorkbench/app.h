#pragma once
#include "app_support.h"
#include "geometry/predicates.h"
class Canvas : public QWidget {
  public:
    std::vector<QPointF> points;
    std::function<void()> changed;
    std::function<void(QPainter &)> overlay;
    int limit = -1, drag = -1;
    QColor pointColor = QColor("#2563eb");
    int answer = 2;
    explicit Canvas(QWidget *parent = nullptr) : QWidget(parent) {
        setMouseTracking(true);
    }

  protected:
    void mousePressEvent(QMouseEvent *e) override {
        if (e->button() != Qt::LeftButton)
            return;
        auto p = mousePosition(e);
        drag = -1;
        for (size_t i = 0; i < points.size(); ++i)
            if (QLineF(p, points[i]).length() < 12) {
                drag = int(i);
                return;
            }
        if (limit < 0 || int(points.size()) < limit) {
            points.push_back(p);
            if (changed)
                changed();
            update();
        }
    }
    void mouseMoveEvent(QMouseEvent *e) override {
        if (drag < 0 || !(e->buttons() & Qt::LeftButton))
            return;
        auto p = mousePosition(e);
        points[size_t(drag)] = {std::clamp(p.x(), 0., double(width())),
                                std::clamp(p.y(), 0., double(height()))};
        if (changed)
            changed();
        update();
    }
    void mouseReleaseEvent(QMouseEvent *) override {
        drag = -1;
    }
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        canvasBackground(painter, rect());
        if (overlay)
            overlay(painter);
        for (size_t i = 0; i < points.size(); ++i) {
            QColor color = pointColor;
            if (limit == 3 && i == 2 && answer != 2)
                color = answer > 0   ? QColor("#16a34a")
                        : answer < 0 ? QColor("#dc2626")
                                     : QColor("#eab308");
            painter.setPen(QPen(color.darker(), 1));
            painter.setBrush(color);
            painter.drawEllipse(points[i], 6, 6);
            painter.setPen(QColor("#0f172a"));
            QString label = limit == 3   ? QString(i == 0   ? "A"
                                                   : i == 1 ? "B"
                                                            : "P")
                            : limit == 4 ? QString(QChar('A' + int(i)))
                                         : QString("P%1").arg(qulonglong(i + 1));
            painter.drawText(points[i] + QPointF(9, -9), label);
        }
    }
};
class TaskWindow : public QMainWindow {
  public:
    Canvas *canvas = new Canvas;
    QCheckBox *online = new QCheckBox("Онлайн-пересчёт");
    QCheckBox *circles = new QCheckBox("Описанные окружности");
    QTextBrowser *result = new QTextBrowser;
    TaskWindow() {
        QVBoxLayout *controls;
        setupWindow(*this, "Предикат поворота", canvas, controls);
        canvas->limit = 3;
        auto *hint = new QLabel("Левая кнопка: добавить точку.\nПотяните точку мышью для "
                                "перемещения.\n«Очистить» — начать заново.");
        hint->setWordWrap(true);
        controls->addWidget(hint);
        online->setChecked(true);
        controls->addWidget(online);
        delete circles;
        circles = nullptr;
        addButton(controls, "Рассчитать", [this]() { calculate(); });
        addButton(controls, "Очистить", [this]() {
            canvas->points.clear();
            invalidate();
            result->setPlainText("Холст очищен.");
        });
        controls->addWidget(result, 1);
        canvas->changed = [this]() {
            if (online->isChecked())
                calculate();
            else {
                invalidate();
                result->setPlainText("Данные изменены. Нажмите «Рассчитать».");
            }
        };
        canvas->overlay = [this](QPainter &p) { draw(p); };
        connect(online, &QCheckBox::toggled, this, [this](bool enabled) {
            if (enabled)
                calculate();
        });
        calculate();
    }
    void invalidate() {
        canvas->answer = 2;
        canvas->update();
    }
    void calculate() {
        canvas->answer = 2;
        if (canvas->points.size() < 3) {
            result->setPlainText("Первые два клика — A и B, третий — P.");
            return;
        }
        auto a = geometry_qt::toGeometryPoint(canvas->points[0]),
             b = geometry_qt::toGeometryPoint(canvas->points[1]),
             p = geometry_qt::toGeometryPoint(canvas->points[2]);
        int sign = geometry::orientation(a, b, p);
        canvas->answer = sign;
        result->setPlainText(
            QString("Ответ: %1\n%2\n\nЗелёный — слева; красный — справа; жёлтый — на прямой.")
                .arg(sign)
                .arg(sign > 0   ? "P слева от AB"
                     : sign < 0 ? "P справа от AB"
                                : "P на прямой AB"));
        if (a.x == b.x && a.y == b.y)
            result->append("A = B: направление не определено, определитель равен нулю.");
        canvas->update();
    }
    void draw(QPainter &p) {
        if (canvas->points.size() >= 2) {
            p.setPen(QPen(QColor("#2563eb"), 3));
            p.drawLine(canvas->points[0], canvas->points[1]);
        }
        if (canvas->points.size() >= 2) {
            auto a = canvas->points[0], b = canvas->points[1];
            QLineF line(a, b);
            if (line.length() > 0) {
                auto v = (b - a) / line.length();
                auto side = QPointF(-v.y(), v.x());
                p.drawLine(b, b - v * 14 + side * 6);
                p.drawLine(b, b - v * 14 - side * 6);
            }
        }
    }
    void smokeTest() {
        simulateClick(canvas, {100, 250});
        simulateClick(canvas, {400, 250});
        simulateClick(canvas, {250, 100});
        ensure(canvas->answer == 1, "screen Y conversion / left turn");
        simulateDrag(canvas, {250, 100}, {250, 350});
        ensure(canvas->answer == -1, "turn while dragging");
        online->setChecked(false);
        simulateDrag(canvas, {250, 350}, {250, 100});
        ensure(canvas->answer == 2, "stale turn hidden");
        calculate();
        ensure(canvas->answer == 1, "manual turn");
    }
};
