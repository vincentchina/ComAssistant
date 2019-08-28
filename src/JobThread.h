#ifndef __JOB_THREAD_H__
#define __JOB_THREAD_H__

#include <QThread>
#include <QList>
#include <QMutex>
#include <QSemaphore>

class JobThread : public QThread
{
    Q_OBJECT
public:
    typedef unsigned int JobId;
    enum{ InvalidID = 0 };
    typedef void (*JobWorker)( void* jobData );
    JobThread();
    ~JobThread();

    JobId startOneJob( JobWorker job, void* jobData, bool mutualJob = true );
    JobId startLoopJob( JobWorker job, void* jobData, bool mutualJob = true );
    JobId startSingleJob( JobWorker job, void* jobData, bool repeat );
    void stopJob( JobId id );
    void stop();
private:
    virtual void run();
    JobId startJob( JobWorker job, void* jobData, bool mutexlJob, bool repeat );

private:
    struct JobData
    {
        JobWorker   job;
        void*       data;
        bool        isRepeat;
        JobId       id;
        JobData() : job(NULL),data(NULL),isRepeat( false ),id( InvalidID )
        {
        }
    };
    QList<JobData>  m_jobList;
    QMutex          m_mutex;
    QSemaphore      m_semaphore;
    bool            m_doJob;
    bool            m_singleJob;
};

#endif
