/****************************************************************/
/****************************************************************/
/************************* Declarations **************************/
/****************************************************************/
/****************************************************************/
#define SERIAL_BAUD_RATE              230400

template<class T>
class auto_ptr {
private:
    T* _t;
public:
    auto_ptr() : _t(0) {
    }

    auto_ptr(T* t) : _t(t) {
    }

    template<typename _T1>
    auto_ptr(auto_ptr<_T1> &__a) : _t(__a.release()) {
    }

    auto_ptr<T> operator=(auto_ptr<T> &p) {
        if (_t) {
            delete _t;
        }

        _t = p.release();
    }

    T* operator->() {
        return this->get();
    }

    ~auto_ptr() {
        if (_t) {
            delete _t;
            _t = NULL;
        }
    }

    T* get() const {
        return _t;
    }

    T* release() {
        T* tmp = _t;
        _t = NULL;

        return tmp;
    }
};


class Element {
public:
    Element* _next;
    Element();
    virtual ~Element();
};

class ListIterator;
class ListElem : public Element {
private:
    Element* _head;
    friend class ListIterator;
public:
    ListElem();
    ~ListElem();
    bool add(auto_ptr<Element>);
    auto_ptr<ListIterator> iterator();
};


class ListIterator {
private:
    ListElem* _owner;
    unsigned int _index;
public:
    ListIterator(ListElem* owner);
    Element* next();
    bool hasNext();

    ListIterator();
    ~ListIterator();
};

class TimerFunction : public Element {
private:
    unsigned long _interval;
    unsigned long _timeToNextRun;
public:
    TimerFunction();
    virtual ~TimerFunction();
    void update(unsigned long elapsed_millis);
    virtual void run(unsigned long elapsed_millis) = 0;
    void setInterval(unsigned long interval, unsigned long elapsed_millis);
};

class TimerSys {
private:
public:
    static ListElem _functions;
    static bool register_func(auto_ptr<TimerFunction> tf, unsigned long interval, unsigned long elapsed_millis);
    static void update(unsigned long elapsed_millis);
};


class InputPinReader : public Element {
private:
    int _pin;
public:
    int _high;
    InputPinReader(int pin);
    virtual ~InputPinReader();
    void run();
};

class ForwardButton : public InputPinReader {
private:
public:
    ForwardButton();
    ~ForwardButton();
};

class BackwardButton: public InputPinReader {
private:
public:
    BackwardButton();
    ~BackwardButton();
};

class LeftButton: public InputPinReader {
private:
public:
    LeftButton();
    ~LeftButton();
};

class RightButton: public InputPinReader {
private:
public:
    RightButton();
    ~RightButton();
};


class Robot : public TimerFunction {
private:

public:
    Robot();
    ~Robot();
    void run(unsigned long elapsed_millis);
};

ListElem TimerSys::_functions;


/****************************************************************/
/****************************************************************/
/************************* Definitions **************************/
/****************************************************************/
/****************************************************************/

Element::Element() : _next(0) { }
Element::~Element() { }

ListElem::ListElem() : _head(0) { }

ListElem::~ListElem() {
    while (_head != NULL) {
        Element* tmp = _head;
        _head = _head->_next;

        delete tmp;
    }
}

bool ListElem::add(auto_ptr<Element> e) {
    if (e.get() == NULL) {
        return false;
    }

    if (_head == NULL) {
        _head = e.release();
        _head->_next = NULL;
    } else {
        Element* runner = _head;
        while (runner->_next != NULL) {
            runner = runner->_next;
        }

        runner->_next = e.release();
        runner->_next->_next = NULL;
    }

    return true;
}

auto_ptr<ListIterator> ListElem::iterator() {
    auto_ptr<ListIterator> r(new ListIterator(this));

    return r;
}

ListIterator::ListIterator(ListElem* owner) : _owner(owner), _index(0) { }
ListIterator::~ListIterator() { }

Element* ListIterator::next() {
    Element* runner = _owner->_head;
    unsigned int i = 0;
    while (runner != NULL) {
        if (i == _index) {
            _index++;
            return runner;
        }

        i++;
        runner = runner->_next;
    }

    return NULL;
}

bool ListIterator::hasNext() {
    Element* runner = _owner->_head;
    unsigned int i = 0;
    while (runner != NULL) {
        if (i == _index) {
            return true;
        }

        i++;
        runner = runner->_next;
    }

    return false;
}

void TimerFunction::setInterval(unsigned long interval, unsigned long elapsed_millis) {
    _interval = interval;
    _timeToNextRun = elapsed_millis + _interval;
}

TimerFunction::TimerFunction() : _interval(0), _timeToNextRun(0) { }
TimerFunction::~TimerFunction() { }

void TimerFunction::update(unsigned long elapsed_millis) {
    if (_interval == 0) {
        run(elapsed_millis);
    } else if (elapsed_millis >= _timeToNextRun) {
        run(elapsed_millis);
        _timeToNextRun += _interval;
    }
}

bool TimerSys::register_func(auto_ptr<TimerFunction> tf, unsigned long interval, unsigned long elapsed_millis) {
    tf->setInterval(interval, elapsed_millis);
    auto_ptr<Element> t(tf.release());
    return TimerSys::_functions.add(t);
}

void TimerSys::update(unsigned long elapsed_millis) {
    auto_ptr<ListIterator> i = TimerSys::_functions.iterator();

    while (i->hasNext()) {
        TimerFunction* tf = static_cast<TimerFunction*>(i->next());
        tf->update(elapsed_millis);
    }
}

InputPinReader::InputPinReader(int pin) : _pin(pin), _high(false) {
    pinMode(_pin, INPUT);
}

InputPinReader::~InputPinReader() { }

void InputPinReader::run() {
    if (digitalRead(_pin) == HIGH) {
        _high = true;
    } else {
        _high = false;
    }
}

ForwardButton::ForwardButton() : InputPinReader(8) { }
ForwardButton::~ForwardButton() { }

BackwardButton::BackwardButton() : InputPinReader(9) { }
BackwardButton::~BackwardButton() { }

LeftButton::LeftButton() : InputPinReader(10) { }
LeftButton::~LeftButton() { }

RightButton::RightButton() : InputPinReader(11) { }
RightButton::~RightButton() { }


Robot::Robot() {
}

Robot::~Robot() { }

void Robot::run(unsigned long elapsed_millis) {
}

InputPinReader* forwardButton;
InputPinReader* backwardButton;
InputPinReader* leftButton;
InputPinReader* rightButton;


void setup(void) { 
    Serial.begin(SERIAL_BAUD_RATE);
    unsigned long elapsed_millis = millis();

    forwardButton = new ForwardButton();
    backwardButton = new BackwardButton();    
    leftButton = new LeftButton();
    rightButton = new RightButton();

    //auto_ptr<TimerFunction> robot(new Robot());

    //TimerSys::register_func(robot, 500, elapsed_millis);


     
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);    

    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);     
}

void moveForward() {
    digitalWrite(2, HIGH);     
    digitalWrite(3, LOW);     
    digitalWrite(4, HIGH);     
    digitalWrite(5, LOW);     
}

void moveBackward() {
    digitalWrite(3, HIGH);     
    digitalWrite(2, LOW);     
    digitalWrite(5, HIGH);     
    digitalWrite(4, LOW);     
}

void moveLeft() {
    digitalWrite(2, HIGH);     
    digitalWrite(3, LOW);     
    digitalWrite(5, HIGH);     
    digitalWrite(4, LOW);     
}

void moveRight() {
    digitalWrite(3, HIGH);     
    digitalWrite(2, LOW);     
    digitalWrite(4, HIGH);     
    digitalWrite(5, LOW);     
}

void stop() {
    digitalWrite(3, HIGH);     
    digitalWrite(2, HIGH);     
    digitalWrite(4, HIGH);     
    digitalWrite(5, HIGH);       
}

void loop(void) {
    unsigned long elapsed_millis = millis();
    Serial.print("checking inputs\n");
    forwardButton->run();
    backwardButton->run();
    leftButton->run();
    rightButton->run();
    delay(10);


    if(forwardButton->_high && !backwardButton->_high && !leftButton->_high && !rightButton->_high) {
      //Serial.print("moveForward\n");      
      moveForward();
    } else if(!forwardButton->_high && backwardButton->_high && !leftButton->_high && !rightButton->_high) {
      //Serial.print("moveBackward\n");            
      moveBackward();
    } else if(!forwardButton->_high && !backwardButton->_high && leftButton->_high && !rightButton->_high) {
      //Serial.print("moveLeft\n");            
      moveLeft();
    }  else if(!forwardButton->_high && !backwardButton->_high && !leftButton->_high && rightButton->_high) {
      //Serial.print("moveRight\n");                  
      moveRight();
    } else {
      stop();
    }
    
    #if 0
    // forward
    digitalWrite(2, LOW);     
    digitalWrite(3, HIGH);     
    delay(2000);

     // open
    digitalWrite(2, LOW);     
    digitalWrite(3, LOW);     
    delay(2000);         

    // reverse
    digitalWrite(2, HIGH);     
    digitalWrite(3, LOW);     
    delay(2000);             
    
    // hold
    digitalWrite(2, HIGH);     
    digitalWrite(3, HIGH);    
    delay(2000);


    // forward
    digitalWrite(4, LOW);     
    digitalWrite(5, HIGH);     
    delay(2000);

     // open
    digitalWrite(4, LOW);     
    digitalWrite(5, LOW);     
    delay(2000);         

    // reverse
    digitalWrite(4, HIGH);     
    digitalWrite(5, LOW);     
    delay(2000);             
    
    // hold
    digitalWrite(4, HIGH);     
    digitalWrite(5, HIGH);    
    delay(2000);    

    #endif
}

