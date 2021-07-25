#include "graph.h"
#include "ui_graph.h"
#include <QtGui>
#include <QtCore>

/*构造函数*/
Graph::Graph(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Graph)
{
    ui->setupUi(this);
    /*初始化界面*/
    setWindowTitle("Shortes Path Of Graph");
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);

    edgeManager.setX(isNotActive);
    edgeManager.setY(isNotActive);
    DijkstraManager.setX(isNotActive);
    DijkstraManager.setY(isNotActive);
    BellmanManager.setX(isNotActive);
    BellmanManager.setY(isNotActive);

    /*初始化控件*/
    //Dijkstra算法求解最短路button
    Dijkstra = new QToolButton(this);
    Dijkstra->setText("Dijkstra");
    Dijkstra->setCheckable(true);  //设置button为选中/不选中两种状态模式
    Dijkstra->adjustSize();  //根据内容调整button大小
    connect(Dijkstra, SIGNAL(toggled(bool)), this, SLOT(resetPath()));
    //Bellman算法求解最短路button
    Bellman = new QToolButton(this);
    Bellman->setText("Floyd");
    Bellman->setCheckable(true);
    Bellman->adjustSize();
    connect(Bellman, SIGNAL(toggled(bool)), this, SLOT(resetPath()));
    //清除界面button
    clear = new QToolButton(this);
    clear->setText("Clear");
    clear->adjustSize();
    connect(clear, SIGNAL(clicked(bool)), this, SLOT(clearAll()));
    //单行文本输入控件
    reader = new QLineEdit(this);
    reader->hide();
    reader->setFixedWidth(40);
    connect(reader, SIGNAL(returnPressed()), this, SLOT(input()));

}

/*设置事件*/
void Graph::paintEvent(QPaintEvent *) {//绘制事件
    //创建画笔
    QPainter Painter(this);
    Painter.setRenderHint(QPainter::Antialiasing); //反走样
    QPen pen(Qt::black); //颜色
    pen.setStyle(Qt::SolidLine); //线型
    pen.setWidth(2); //
    Painter.setPen(pen);

    //设置字体
    QFont font;
    font.setWeight(5); //字体粗细
    font.setPixelSize(10); //像素值
    Painter.setFont(font);

    //设置颜色
    QColor* dijkstraColor = new QColor(0, 0, 150, 255);
    QColor* bellmanColor = new QColor(0, 150, 0, 255);

    /*绘制顶点：以一个圆形表示图的顶点*/
    //边缘
    for(int i = 0; i < position.size(); i++) {
        if(position[i].x() == erased)
            continue;
        Painter.drawEllipse(position[i], radius, radius); //绘制以左上顶点为position[i]的矩形为外切矩形的圆形表示顶点
    }
    //内部和序号
    Painter.setBrush(palette().dark());  //设置画刷的颜色
    bool vertexOfDijkstra = false;
    bool vertexOfBellman = false;
    for(int i = 0; i < position.size(); i++) {
        if(position[i].x() == erased)
            continue;
        if(Dijkstra->isChecked()) {
            for(int j = 0; j < DijkstraPath.size(); j++) {
                if(DijkstraPath[j] == i) {
                    vertexOfDijkstra = true;
                    break;
                }
            }
            if(vertexOfDijkstra)
                Painter.setBrush(*dijkstraColor); //blue
        }
        if(Bellman->isChecked()) {
            for(int j = 0; j < BellmanPath.size(); j++) {
                if(BellmanPath[j] == i) {
                    vertexOfBellman = true;
                    break;
                }
            }
            if(vertexOfBellman)
                Painter.setBrush(*bellmanColor); //green
        }

        Painter.drawEllipse(position[i], radius - radius / 5, radius - radius / 5);
        Painter.drawText(position[i].x() - radius, position[i].y() - radius, 2 * radius, 2 * radius, Qt::AlignCenter, QString::number(i));
        if(vertexOfDijkstra || vertexOfBellman) {
            Painter.setBrush(palette().dark());
            vertexOfDijkstra = false;
            vertexOfBellman = false;
        }
    }

    //绘制被选中的结点(赋权值)
    if(edgeManager.x() == isActive) {
        Painter.setBrush(palette().window());
        Painter.drawEllipse(position[edgeManager.y()], radius, radius);
        Painter.setBrush(Qt::red);
        Painter.drawEllipse(position[edgeManager.y()], radius - radius / 5, radius - radius / 5);
        Painter.drawText(position[edgeManager.y()].x() - radius, position[edgeManager.y()].y() - radius, 2 * radius, 2 * radius, Qt::AlignCenter, QString::number(edgeManager.y()));
    }

    //绘制被选中作为求最短路的起始顶点
    if(DijkstraManager.x() != isNotActive && DijkstraManager.y() == isNotActive) {
        Painter.setBrush(*dijkstraColor);
        Painter.drawEllipse(position[DijkstraManager.x()], radius - radius / 5, radius - radius / 5);
        Painter.drawText(position[DijkstraManager.x()].x() - radius, position[DijkstraManager.x()].y() - radius, 2 * radius, 2 * radius, Qt::AlignCenter, QString::number(DijkstraManager.x()));
    }
    if(BellmanManager.x() != isNotActive && BellmanManager.y() == isNotActive) {
        Painter.setBrush(*bellmanColor);
        Painter.drawEllipse(position[BellmanManager.x()], radius - radius / 5, radius - radius / 5);
        Painter.drawText(position[BellmanManager.x()].x() - radius, position[BellmanManager.x()].y() - radius, 2 * radius, 2 * radius, Qt::AlignCenter, QString::number(BellmanManager.x()));
    }

    /*绘制边*/
    QLineF one,tow,three,neoLine,arrowBase;
    QPointF prime;
    bool oneIsInDijkstra = false, bothAreInDijkstra = false;
    bool oneIsInBellman = false, bothAreInBellman = false;
    int vertex1positionInDijkstra, vertex2positionInDijkstra;
    int vertex1positionInBellman, vertex2positionInBellman;
    for(int i = 0; i < position.size(); i++) {
        if(position[i].x() == erased)
            continue;

        for(int j = 0; j < DijkstraPath.size(); j++) {
            if(DijkstraPath[j] == i) {
                oneIsInDijkstra = true;
                vertex1positionInDijkstra = j;
                break;
            }   
        }
        for(int j = 0; j < BellmanPath.size(); j++) {
            if(BellmanPath[j] == i) {
                oneIsInBellman = true;
                vertex1positionInBellman = j;
                break;
            }
        }

        for(int j = 0; j < matrix[i].size(); j++) {
            if(position[matrix[i][j]].x() == erased)
                continue;
            for(int k = 0; k < DijkstraPath.size(); k++) {
                if((DijkstraPath[k] == matrix[i][j]) && oneIsInDijkstra) {
                    bothAreInDijkstra = true;
                    vertex2positionInDijkstra = k;
                    break;
                }    
            }
            for(int k = 0; k < BellmanPath.size(); k++) {
                if((BellmanPath[k] == matrix[i][j]) && oneIsInBellman) {
                    bothAreInBellman = true;
                    vertex2positionInBellman = k;
                    break;
                }
            }

            if(bothAreInDijkstra && ((vertex1positionInDijkstra == vertex2positionInDijkstra + 1) || (vertex2positionInDijkstra == vertex1positionInDijkstra + 1))) {
                Painter.setPen(*dijkstraColor);
            }

            else if(bothAreInBellman && ((vertex1positionInBellman == vertex2positionInBellman + 1) || (vertex2positionInBellman == vertex1positionInBellman + 1))) {
                Painter.setPen(*bellmanColor);
            }

            one.setP1(position[i]);
            one.setP2(position[matrix[i][j]]);
            one.setLength(one.length() - radius);
            tow.setP2(position[i]);
            tow.setP1(position[matrix[i][j]]);
            tow.setLength(tow.length() - radius);

            prime.setX(one.p2().x());
            prime.setY(one.p2().y());
            neoLine.setP1(tow.p2());
            neoLine.setP2(prime);
            neoLine.setLength(neoLine.length()-10);


            arrowBase = neoLine.normalVector();
            arrowBase.translate(neoLine.dx(),neoLine.dy());
            arrowBase.setLength(5);
            three.setP1(arrowBase.p2());
            three.setP2(neoLine.p2());
            three.setLength(10);

            Painter.drawLine(neoLine);
            Painter.drawLine(three.p1(), three.p2());
            Painter.drawLine(three.p1(), prime);
            Painter.drawLine(three.p2(), prime);

            if(bothAreInDijkstra)
            {
                Painter.setPen(Qt::black);
                bothAreInDijkstra = false;
            }
            if(bothAreInBellman) {
                Painter.setPen(Qt::black);
                bothAreInBellman = false;
            }

        }

        oneIsInDijkstra = false;
        oneIsInBellman = false;
    }

    /*设置权重*/
    for(int i = 0; i < position.size(); i++) {
        if(position[i].x() == erased)
            continue;
        for(int j = 0; j < weight[i].size(); j++) {
            if(position[matrix[i][j]].x() == erased)
                continue;
            Painter.save();
            Painter.translate(position[i]);
            Painter.rotate(-angle[i][j]);
            Painter.drawText(radius + 20, 10, QString::number(weight[i][j]));
            Painter.restore();
        }
    }
}

/*实现事件函数*/
//鼠标单击事件
void Graph::mousePressEvent(QMouseEvent *event) {
    if(reader->isVisible()) {
        reader->setFocus();
        return;
    }
    QRect rec(margin + radius, margin + radius, width() - 2 * margin - 2 * radius, height() - 2 * margin - 2 * radius);

    //单击鼠标左键，增加一个顶点或者是选择一个已有的顶点或者选中button
    if(event->button() == Qt::LeftButton) {
        if((DijkstraManager.x() != isNotActive) && (DijkstraManager.y() != isNotActive)) {//Dijkstra button
            Dijkstra->setFocus();
            return;
        }

        if((BellmanManager.x() != isNotActive) && (BellmanManager.y() != isNotActive)) {//Bellman button
            Bellman->setFocus();
            return;
        }

        if(rec.contains(event->pos())) {
            if(position.isEmpty()) {//生成第一个结点，并记录他的位置
                position.append(event->pos());
            } else {
                bool isClicked = false;
                int theClicked = 0;
                QRect clickArea;

                while(!isClicked) { //选中了一个已有结点，将其状态标记为被选中
                    clickArea.setX(position[theClicked].x() - 2 * radius);
                    clickArea.setY(position[theClicked].y() - 2 * radius);
                    clickArea.setWidth(4 * radius);
                    clickArea.setHeight(4 * radius);
                    if(clickArea.contains(event->pos())) {
                        isClicked = true;
                        break;
                    }
                    theClicked++;
                    if(theClicked == position.size())
                        break;
                }

                if(isClicked == false) { //没有选中任何结点，点击了空白处，则增加一个节点
                    if(Dijkstra->isChecked() || Bellman->isChecked() || edgeManager.x() == isActive)
                        return;

                    position.append(event->pos());
                    QVector<int> vec;
                    QVector<qreal> vec2;
                    matrix.insert(position.size(), vec);
                    weight.insert(position.size(), vec);
                    angle.insert(position.size(), vec2);
                } else { //选中了一个顶点
                    clickArea.setX(position[theClicked].x() - radius);
                    clickArea.setY(position[theClicked].y() - radius);
                    clickArea.setWidth(2 * radius);
                    clickArea.setHeight(2 * radius);
                    if(clickArea.contains(event->pos())) {
                        if(Dijkstra->isChecked()) {
                            if(DijkstraManager.x() == isNotActive) {
                                DijkstraManager.setX(theClicked);
                            } else if(DijkstraManager.x() == theClicked) {
                                DijkstraManager.setX(isNotActive);
                            } else {
                                DijkstraManager.setY(theClicked);

                                bellman();
                            }
                        } else if(Bellman->isChecked()) {
                            if(BellmanManager.x() == isNotActive) {
                                BellmanManager.setX(theClicked);
                            } else if(BellmanManager.x() == theClicked) {
                                BellmanManager.setX(isNotActive);
                            } else {
                                BellmanManager.setY(theClicked);
                                floyd();
                            }

                        } else {
                            if(edgeManager.x() == isNotActive) {
                                edgeManager.setX(isActive);
                                edgeManager.setY(theClicked);
                            } else if(edgeManager.x() == isActive) {
                                if(edgeManager.y() == theClicked) { //点击同一个两次，则取消选中
                                    edgeManager.setX(isNotActive);
                                } else { //增加一条边
                                    int k = 0;
                                    bool isExist = false;
                                    while(k < matrix[edgeManager.y()].size()) {
                                        if(matrix[edgeManager.y()][k] == theClicked) {
                                            isExist = true;
                                            break;
                                        }
                                        k++;
                                    }

                                    if(!isExist) {
                                        QLineF line(position[edgeManager.y()], position[theClicked]);
                                        matrix[edgeManager.y()].append(theClicked);
                                        angle[edgeManager.y()].append(line.angle());
                                        reader->move((position[edgeManager.y()].x() + position[theClicked].x()) / 2, (position[edgeManager.y()].y() + position[theClicked].y()) / 2);
                                        reader->setEnabled(true);
                                        reader->setVisible(true);
                                        reader->setFocus();
                                        edgeManager.setX(isNotActive);
                                    } else {
                                        edgeManager.setX(isNotActive);
                                    }

                                }
                            }
                        }
                    }
                }
            }
        }

    }

    if(event->button() == Qt::RightButton) {
        bool isCliked = false;
        int theClicked = 0;
        QRect clickedArea;
        while (!isCliked) {
            clickedArea.setX(position[theClicked].x() - radius);
            clickedArea.setY(position[theClicked].y() - radius);
            clickedArea.setWidth(2 * radius);
            clickedArea.setHeight(2 * radius);
            if(clickedArea.contains(event->pos())) {
                isCliked = true;
                break;
            }
            theClicked++;
            if(theClicked == position.size())
                break;
        }
        if(isCliked) {
            position[theClicked].setX(erased);
        }
        if(Dijkstra->isChecked()) {

            bellman();
        }
        if(Bellman->isChecked()) {
           floyd();
        }
    }

    update();
}

//鼠标移动事件
void Graph::mouseMoveEvent(QMouseEvent *event) {
    if(reader->isVisible()) {
        reader->setFocus();
        return;
    }
    QRect rect(radius + margin/2, radius + margin/2, width() - 2 * radius - margin, height() - 2 * radius - margin);
    if(rect.contains(event->pos())) {//单击
        bool isChecked = false;
        int theChecked = 0;
        QRect safeArea;

        while(!isChecked) {
            safeArea.setX(position[theChecked].x() - radius);
            safeArea.setY(position[theChecked].y() - radius);
            safeArea.setWidth(2 * radius);
            safeArea.setHeight(2 * radius);
            if(safeArea.contains(event->pos())) {
                isChecked = true;
                break;
            }
            theChecked++;
            if(theChecked == position.size())
                break;
        }

        if(isChecked) {
            position[theChecked] = event->pos();
            for(int i = 0; i < angle[theChecked].size(); i++) {
                QLineF line(position[theChecked], position[matrix[theChecked][i]]);
                angle[theChecked][i] = line.angle();
            }
            edgeManager.setX(isNotActive);
            update();
        }
    }


}

void Graph::resizeEvent(QResizeEvent*) { //调整button布局
    int y = height() - Dijkstra->height() - 10;
    clear->move(10, y);
    Dijkstra->move(20 + clear->width(), y);
    Bellman->move(100 + Dijkstra->width(), y);
    update();
}

/*实现槽函数*/
//输入权重
void Graph::input() {
    weight[edgeManager.y()].append(reader->text().toInt());
    reader->clear();
    reader->setDisabled(true);
    reader->hide();
    update();

}

//重置最短路
void Graph::resetPath() {
    DijkstraManager.setX(isNotActive);
    DijkstraManager.setY(isNotActive);
    BellmanManager.setX(isNotActive);
    BellmanManager.setY(isNotActive);
    DijkstraPath.clear();
    BellmanPath.clear();
    update();

}

//清除界面
void Graph::clearAll() {
    position.clear();
    matrix.clear();
    weight.clear();
    angle.clear();
    DijkstraPath.clear();
    BellmanPath.clear();
    DijkstraManager.setX(isNotActive);
    DijkstraManager.setY(isNotActive);
    BellmanManager.setX(isNotActive);
    BellmanManager.setY(isNotActive);
    edgeManager.setX(isNotActive);
    edgeManager.setY(isNotActive);

    if(Dijkstra->isChecked())
        Dijkstra->toggle();
    if(Bellman->isChecked())
        Bellman->toggle();

    update();
}



//Floyd-Warshall算法求最短路,使用邻接矩阵来存储图
//基本思想：任意两点之间的最短路，如果允许若干个中间结点
//遍历这些可以经过的中间结点,来更新两点间的最短路
void Graph::floyd() {
    BellmanPath.clear();

    int numOfVer = position.size();
    int edge[numOfVer][numOfVer];  //邻接矩阵
    int parent[numOfVer][numOfVer]; //parent[i][j]表示从结点i到结点j的路径上，j的前驱节点为结点parent[i][j]

    //初始化邻接矩阵，使各路径均为无限大
    //初始化前驱节点矩阵，使节点i到结点j的路径上j的前驱节点均为i
    for(int i = 0; i < numOfVer; i++) {
        for(int j = 0; j < numOfVer; j++) {
            edge[i][j] = inf;
            parent[i][j] = i;
        }
    }

    //更新邻接矩阵，使用邻接矩阵存储图

    for(int i = 0; i < numOfVer; i++){
        if(position[i].x() == erased)
            continue;
        for(int j = 0; j < matrix[i].size(); j++) {
            if(position[matrix[i][j]].x() == erased)
                continue;
            edge[i][matrix[i][j]] = weight[i][j];

        }
    }



    //Floyd算法的核心代码
    for(int i = 0; i < numOfVer; i++) {//第一层循环，表示分别可以经过0, 1，2，……，n-1等中间结点
        for(int j = 0; j < numOfVer; j++) { //第二层循环，源节点为结点j
            for(int k = 0; k < numOfVer; k++) { //第三层循环，目的结点为结点k
                if(edge[j][k] > edge[j][i] + edge[i][k]){
                    edge[j][k] = edge[j][i] + edge[i][k];
                    parent[j][k] = parent[i][k];
                }
            }

        }

    }

    if(edge[BellmanManager.x()][BellmanManager.y()] != inf) {
        int i = BellmanManager.y();
        while(i != BellmanManager.x()) {
            BellmanPath.append(i);
            i = parent[BellmanManager.x()][i];
        }
        BellmanPath.append(BellmanManager.x());
    } else {
        BellmanPath.append(inf);
    }

    update();

}

//dijkstra算法求最短路
void Graph::dijkstra() {
     DijkstraPath.clear();

     int numOfVer = position.size();
     int edge[numOfVer][numOfVer];  //邻接矩阵
     int parent[numOfVer][numOfVer];  //前驱节点
     int dis[numOfVer][numOfVer];   //最短距离矩阵
     int flag[numOfVer];  //标记数组

     //初始化邻接矩阵和前驱节点
     for(int i = 0; i < numOfVer; i++) {
         for(int j = 0; j < numOfVer; j++) {
             parent[i][j] = i;
             if(i == j) {
                 edge[i][j] = 0;
                 dis[i][j] = 0;
             }
             else {
                 edge[i][j] = inf;
                 dis[i][j] = inf;
             }
         }
     }

     //根据边的信息更新邻接矩阵和最短距离矩阵
     for(int i = 0; i < numOfVer; i++){
         if(position[i].x() == erased)
             continue;
         for(int j = 0; j < matrix[i].size(); j++) {
             if(position[matrix[i][j]].x() == erased)
                 continue;
             edge[i][matrix[i][j]] = weight[i][j];
             dis[i][matrix[i][j]] = weight[i][j];

         }
     }

     //dijkstra算法核心代码
     for(int k = 0; k < numOfVer; k++) {//分别以n个结点作为源节点
         for(int i = 0; i < numOfVer; i++)
             flag[i] = 0;
         flag[k] = 1;

         for(int i = 1; i < numOfVer; i++) { //找到源节点到另外结点的最短路
             int min = inf;
             int index;
             for(int j = 0; j < numOfVer; j++) { //寻找最小的权值（边）
                 if(flag[j] == 0 && dis[k][j] < min) {
                     min = dis[k][j];  //记录最小权值
                     index = j;       //记录最小权值对应的结点
                 }
             }
             flag[index] = 1;   //标记找到的该结点
             for(int j = 0; j < numOfVer; j++) { //更新dis矩阵和parent矩阵
                 if((flag[j] == 0) && edge[index][j] != inf && (dis[k][j] > min + edge[index][j])) {
                     dis[k][j] = min + edge[index][j];
                     parent[k][j] = index;
                 }
             }
         }
     }

     if(edge[DijkstraManager.x()][DijkstraManager.y()] != inf) {
         int i = DijkstraManager.y();
         while(i != DijkstraManager.x()) {
             BellmanPath.append(i);
             i = parent[DijkstraManager.x()][i];
         }
         DijkstraPath.append(DijkstraManager.x());
     } else {
         DijkstraPath.append(inf);
     }

     update();
}

void Graph::bellman() {
    DijkstraPath.clear();

    int numOfVer = position.size();
    int edge[numOfVer][numOfVer];  //邻接矩阵
    int parent[numOfVer][numOfVer]; //parent[i][j]表示从结点i到结点j的路径上，j的前驱节点为结点parent[i][j]

    for(int i = 0; i < numOfVer; i++) {
        for(int j = 0; j < numOfVer; j++) {
            edge[i][j] = inf;
            parent[i][j] = i;
        }
    }

    for(int i = 0; i < numOfVer; i++){
        if(position[i].x() == erased)
            continue;
        for(int j = 0; j < matrix[i].size(); j++) {
            if(position[matrix[i][j]].x() == erased)
                continue;
            edge[i][matrix[i][j]] = weight[i][j];

        }
    }


    for(int i = 0; i < numOfVer; i++) {
        for(int j = 0; j < numOfVer; j++) {
            for(int k = 0; k < numOfVer; k++) {
                if(edge[j][k] > edge[j][i] + edge[i][k]){
                    edge[j][k] = edge[j][i] + edge[i][k];
                    parent[j][k] = parent[i][k];
                }
            }

        }

    }

    if(edge[DijkstraManager.x()][DijkstraManager.y()] != inf) {
        int i = DijkstraManager.y();
        while(i != DijkstraManager.x()) {
            DijkstraPath.append(i);
            i = parent[DijkstraManager.x()][i];
        }
        DijkstraPath.append(DijkstraManager.x());
    } else {
        DijkstraPath.append(inf);
    }

    update();

}




/*析构函数*/
Graph::~Graph()
{
    delete ui;
}
