#ifndef GRAPH_H
#define GRAPH_H

#include <QMainWindow>
#include <QWidget>
#include <QVector>
#include <QMap>
#include <QToolButton>
#include <QLineEdit>

namespace Ui {
class Graph;
}

class Graph : public QMainWindow
{
    Q_OBJECT

public:
    explicit Graph(QWidget *parent = nullptr); //构造函数
    ~Graph(); //析构函数

private:
    enum {margin = 5, radius = 30, isNotActive = -2, isActive = -1,  erased = -99, inf = 1000000};
    //声明button
    QToolButton* Dijkstra;
    QToolButton* Bellman;
    QToolButton* clear;
    QLineEdit* reader; //单行文本输入，用于输入权值
    QMap<int, QVector<int>> matrix;
    QMap<int, QVector<int>> weight;
    QMap<int, QVector<qreal>> angle;
    QVector<int> DijkstraPath;
    QVector<int> BellmanPath;
    QVector<QPointF> position;
    QPoint edgeManager;
    QPoint DijkstraManager;
    QPoint BellmanManager;

    //声明事件
protected:
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

    //声明槽
public slots:
    void dijkstra();  //Dijkstra算法
    void floyd();   //Bellman-Ford算法
    void bellman();
private slots:
    void input();
    void clearAll();
    void resetPath();


private:
    Ui::Graph *ui;
};

#endif // GRAPH_H
