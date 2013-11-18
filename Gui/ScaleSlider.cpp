//  Natron
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
*Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012. 
*contact: immarespond at gmail dot com
*
*/

#include "ScaleSlider.h"

#include <cassert>
#include <qlayout.h>
#if QT_VERSION < 0x050000
CLANG_DIAG_OFF(unused-private-field);
#include <QtGui/qmime.h>
CLANG_DIAG_ON(unused-private-field);
#endif
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>


#define BORDER_OFFSET 4
#define TICK_HEIGHT 7

ScaleSlider::ScaleSlider(double bottom, double top, int nbValues, double initialPos, Natron::Scale_Type type, QWidget* parent):
QWidget(parent)
, _minimum(bottom)
, _maximum(top)
, _nbValues(nbValues)
, _type(type)
, _position(initialPos)
, _values()
, _displayedValues()
, _XValues()
, _dragging(false)
, _font(new QFont("Times",8))
{

    setContentsMargins(0, 0, 0, 0);
    updateScale();
    setMinimumWidth(150);
}

ScaleSlider::~ScaleSlider(){
    delete _font;
}

static void fitInGui(double size,
                     const QFontMetrics& fontMetrics,
                     const std::vector<double>& values,
                     std::vector<double>* valuesToDisplay,
                     std::vector<double>* coordValues){
    if(values.empty())
        return;
    int precision = 0;
    if(values.size() > 1 &&  values[1] - values[0] < 1){
        precision = 1;
    }
    valuesToDisplay->push_back(values.front());
    double lastDisplayedValueWidth = fontMetrics.width(QString::number(values.front(),'f',precision));
    double widthOfLastValue = fontMetrics.width(QString::number(values.back(),'f',precision));
    coordValues->push_back(0);
    double lastDisplayedValueCoord = 0;
    double coordIncrement = size/(double)(values.size()-1);
    double coord = coordIncrement;
    
    
   
    for(unsigned int i = 1; i < values.size();++i) {
        coordValues->push_back(coord);
        int offsetFromLastValue = lastDisplayedValueWidth;
        //if the number is separated from the prev of an offset
        // and the number is not too close of the last
        if(((coord - lastDisplayedValueCoord) > offsetFromLastValue*2)
           && (lastDisplayedValueCoord + widthOfLastValue*3 <= size)){
            valuesToDisplay->push_back(values[i]);
            lastDisplayedValueCoord = coord;
            lastDisplayedValueWidth = fontMetrics.width(QString::number(values[i],'f',precision));
        }
        coord += coordIncrement;
    }
    valuesToDisplay->push_back(values.back());
    assert(coordValues->size() == values.size());
}

void ScaleSlider::logScale(int N, // number of elements
                           double minimum, //minimum value
                           double maximum, //maximum value
                           double power,//the power used to compute the logscale
                           std::vector<double>* values){
    
    for(int i = 0;i < N;++i) {
        values->push_back(std::pow((double)i,power)*(maximum - minimum)/(std::pow((double)(N-1),power)) + minimum);
    }
   
}

void ScaleSlider::linearScale(int N, // number of elements
                           double minimum, //minimum value
                           double maximum, //maximum value
                           std::vector<double>* values){
    
    double v = minimum;
    for(int i = 0; i < N;++i) {
        values->push_back(v);
        v +=(maximum - minimum)/(double)(N-1);
        
    }
}


void ScaleSlider::updateScale(){
    _values.clear();
    _displayedValues.clear();
    _XValues.clear();
    if (_type == Natron::LINEAR_SCALE) {
        linearScale(_nbValues, _minimum, _maximum,&_values);
    }else if(_type == Natron::LOG_SCALE){
        logScale(_nbValues, _minimum, _maximum,3.,&_values);
    }else if(_type == Natron::EXP_SCALE){
        logScale(_nbValues, _minimum, _maximum,1./2.2,&_values);
    }
    QFontMetrics m(*_font);
    fitInGui(size().width(),m,_values,&_displayedValues,&_XValues);
}
void ScaleSlider::paintEvent(QPaintEvent *e){
    updateScale();
    QFontMetrics metrics(*_font);
    int borderHeight = height() - metrics.height() - 4;
    Q_UNUSED(e);
    QColor bg(50,50,50,255);
    int w = size().width();
    int h = size().height();
    QColor scaleColor(100,100,100);
    QPainter p(this);
    
    //fill the widget with the background color
    p.setWindow(0, 0, w, h);
    p.fillRect(0, 0, w, h, bg);
    
    p.setPen(scaleColor);
    p.setFont(*_font);

    p.drawLine(BORDER_OFFSET,borderHeight,w-BORDER_OFFSET,borderHeight); // horizontal line
    
    
    int y = height()-4;

    // drawing ticks & sub-ticks
    int precision = 0;
    if(_displayedValues.size() > 1 && _displayedValues[1] - _displayedValues[0] < 1){
        precision = 1;
    }
    for(unsigned int i = 0;i < _displayedValues.size();++i) {
        QString numberStr = QString::number(_displayedValues[i],'f',precision);
        if(numberStr.size() > 4){
            numberStr = QString::number(_displayedValues[i],'g',precision);
        }
        double coord = toWidgetCoords(_displayedValues[i]);
        double xPos = coord;
        if(i > 0){
            int sizeOfStr = metrics.width(numberStr);
            if(i != _displayedValues.size() -1){
                xPos = coord - sizeOfStr/2;
            }else{
                xPos = coord - sizeOfStr;
            }
        }
        p.drawText(QPointF(xPos,y), numberStr);
        p.drawLine(coord,2+borderHeight,coord,borderHeight-TICK_HEIGHT);
        
    }
    //drawing cursor
    int cursorPos = (int)(toWidgetCoords(_position) + 0.5);
    p.fillRect(cursorPos-2, borderHeight-TICK_HEIGHT, 4, 2*TICK_HEIGHT,QColor(97,83,30));
    p.setPen(Qt::black);
    p.drawRect(cursorPos-3,borderHeight-TICK_HEIGHT-1,5,2*TICK_HEIGHT+2);
}


void ScaleSlider::mousePressEvent(QMouseEvent* e){
    if(!isEnabled())
        return;
    _position = toScalePosition(e->x());
    _dragging=true;
    emit positionChanged(_position);
    repaint();
    
    
}
void ScaleSlider::mouseMoveEvent(QMouseEvent* e){
    if(_dragging){
        _position = toScalePosition(e->x());
        emit positionChanged(_position);
        repaint();
    }
}
void ScaleSlider::seekScalePosition(double d){
    if(!isEnabled())
        return;
    bool loop = true;
    if(d < _values [0]){
        _position= _values[0];
        loop=false;
    }
    if(d > _values[_values.size()-1]){
        _position= _values[_values.size()-1];
        loop=false;
    }
    if(loop){
        for(unsigned int i=0;i<_values.size();++i) {
            if(i<_values.size()-1 &&  d >= _values[i] && d < _values[i+1]){
                _position= _values[i];
                break;
            }
            if(i == _values.size()-1 ){
                _position=  _values[i];
                break;
            }
        }
    }
    repaint();
}

void ScaleSlider::mouseReleaseEvent(QMouseEvent* e){
    Q_UNUSED(e);
    _dragging=false;
}

double ScaleSlider::toScalePosition(double pos){
    if(pos <= _XValues[0])
        return _values[0];
    if(pos >= _XValues[_XValues.size()-1])
        return _values[_XValues.size()-1];
    for(unsigned int i = 0 ; i < _values.size() ; ++i) {
        if(i<_XValues.size()-1 &&  pos >= _XValues[i] && pos < _XValues[i+1]){
            return _values[i];
        }
        if(i == _XValues.size()-1 ){
            return  _values[i];
            
        }
    }
    return -1;
}

double ScaleSlider::toWidgetCoords(double pos){
    
    if(pos < _values[0]){
        return _XValues[0];
        
    }
    if(pos > _values[_values.size()-1]){
        return _XValues[_XValues.size()-1];
        
    }
    
    for(unsigned int i=0;i<_values.size();++i) {
        if(i<_values.size()-1 &&  pos >= _values[i] && pos < _values[i+1]){
            return  _XValues[i];
            
        }
        if(i == _values.size()-1 ){
            return  _XValues[i];
            
        }
    }
    return -1;
}


// See http://lists.gnu.org/archive/html/octave-bug-tracker/2011-09/pdfJd5VVqNUGE.pdf

void ScaleSlider::LinearScale1(double xmin,double xmax,int n,double* xminp,double* xmaxp,double *dist){
    static double vint[4] = { 1.f,2.f,5.f,10.f };
    static double sqr[3] = { 1.414214f,3.162278f,7.071068f };
    
    if(xmax <= xmin || n == 0) //improper range
        return;
    
    double del =  2e-5f;
    double a = (xmax - xmin) / (double)n;
    double al = log10(a);
    int nal = al;
    if(a < 1.){
        --nal;
    }
    a /= pow(10.,nal);
    int i = 0;
    while(a > sqr[i] && i < 3){
        ++i;
    }
    *dist =  vint[i] * pow(10.,nal);
    double fm1 = xmin / *dist;
    int m1 = fm1;
    if(fm1 < 0.)
        --m1;
    double r_1 = m1 + 1. - fm1;
    if(abs(r_1) <  del){
        ++m1;
    }
    *xminp = *dist * m1;
    double fm2 = xmax / *dist;
    int m2 = fm2 + 1.;
    if(fm2 <  -1.){
        --m2;
    }
    r_1 = fm2 + 1. - m2;
    if(abs(r_1) < del){
        --m2;
    }
    *xmaxp = *dist * m2;
    if(*xminp > xmin){
        *xminp = xmin;
    }
    if(*xmaxp < xmax){
        *xmaxp = xmax;
    }
}

void ScaleSlider::LinearScale2(double xmin,double xmax,int n,double* xminp,double* xmaxp,double *dist){
    static double vint[3] = {1., /*2.,*/5.,10./*,20.*/ };
    
    if(xmax <= xmin || n == 0) //improper range
        return;
    
    double del =  2e-5f;
    double a = (xmax - xmin) / (double)n;
    double al = log10(a);
    int nal = al;
    if(a < 1.){
        --nal;
    }
    a /= pow(10.,nal);
    int i = 0;
    int np  = 0;
    do{
        while(a >= (vint[i]+del) && i < 3){
            ++i;
        }
        *dist =  vint[i] *  pow(10.,nal);
        double fm1 = xmin / *dist;
        int m1 = fm1;
        if(fm1 < 0.)
            --m1;
        double r_1 = m1 + 1. - fm1;
        if(abs(r_1) <  del){
            ++m1;
        }
        *xminp = *dist * (double)m1;
        double fm2 = xmax / *dist;
        int m2 = fm2 + 1.;
        if(fm2 <  -1.){
            --m2;
        }
        r_1 = fm2 + 1. - m2;
        if(abs(r_1) < del){
            --m2;
        }
        *xmaxp = *dist * (double)m2;
        np = (m2 - m1);
        ++i;
    }while(np > n);
    int nx = (n - np) / 2;
    *xminp -= (double)nx * *dist;
    *xmaxp = *xminp + (double)n * *dist;
    
    if (*xminp > xmin) {
        *xminp = xmin;
    }
    if (*xmaxp < xmax) {
        *xmaxp = xmax;
    }
}

void ScaleSlider::LogScale1(double xmin,double xmax,int n,double* xminp,double* xmaxp,double *dist){
    static double vint[11] = { 10.f,9.f,8.f,7.f,6.f,5.f,4.f,3.f,2.f,1.f,.5f };
    
    if(xmax <= xmin || n <= 1 || xmin <= 0.) //improper range
        return;
    
    double del =  2e-5f;
    double xminl = log10(xmin);
    double xmaxl = log10(xmax);
    double a = (xmaxl - xminl) / (double)n;
    double al = log10(a);
    int nal = al;
    if(a < 1.f){
        --nal;
    }
    
    a /= pow(10.,nal);
    int i = 0;
    while(a >= 10. / vint[i] + del && i < 9){
        ++i;
    }
    int i_1 = nal + 1;
    int np = 0;
    double distl;
    do{
        distl = pow(10.,i_1) / vint[i];
        double fm1 = xminl / distl;
        int m1 = fm1;
        if(fm1 < 0.){
            --m1;
        }
        double r_1 = (double)m1 + 1. - fm1;
        if(abs(r_1) < del){
            ++m1;
        }
        *xminp = distl * (double)m1;
        double fm2 = xmaxl / distl;
        int m2 = fm2 + 1.;
        if (fm2 < -1.) {
            --m2;
        }
        r_1 = fm2 + 1. - (double)m2;
        if(abs(r_1) < del){
            --m2;
        }
        *xmaxp = distl * (double)m2;
        
        np = m2 - m1;
        ++i;
    }while(np > n);
    
    int nx = (n - np) / 2;
    *xminp -= (double)nx * distl;
    *xmaxp = *xminp + (double)n * distl;
    
    *dist = pow(10.,distl);
    *xminp = pow(10,*xminp);
    *xmaxp = pow(10,*xmaxp);
    
    if(*xminp > xmin){
        *xminp = xmin;
    }
    if(*xmaxp < xmax){
        *xmaxp = xmax;
    }
}
