namespace Corbot {
    class DeltaTime {
    private:
        struct timeb _startTime;
    public:
        DeltaTime();
        virtual ~DeltaTime();
        void reset();
        double getDeltaMs();
    };

    static double deltaTime(const timeb &before, const timeb &after) {
        double bTime = (before.time * 1000.0) + static_cast<double>(before.millitm);
        double aTime = (after.time * 1000.0) + static_cast<double>(after.millitm);
        // return in milliseconds.
        return aTime - bTime;
    }


    void DeltaTime::reset() {
        srand(time(0));
        ftime(&_startTime);
    }

    DeltaTime::DeltaTime() {
        reset();
    }

    DeltaTime::~DeltaTime() {
    }

    double DeltaTime::getDeltaMs() {
        struct timeb after;
        ftime(&after);

        // return in milliseconds.
        return deltaTime(_startTime, after);
    }
}
