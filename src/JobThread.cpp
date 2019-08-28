#include "JobThread.h"

static JobThread::JobId g_jobIdProvider = 0;

JobThread::JobThread() : QThread()
{
    m_doJob = false;
    m_singleJob = false;
}

JobThread::~JobThread()
{
    stop();
}

JobThread::JobId JobThread::startOneJob( JobWorker job, void* jobData, bool mutualJob )
{
    return startJob( job, jobData, mutualJob, false );
}

JobThread::JobId JobThread::startLoopJob( JobWorker job, void* jobData, bool mutualJob )
{
    return startJob( job, jobData, mutualJob, true );
}

JobThread::JobId JobThread::startSingleJob( JobWorker job, void* jobData, bool repeat )
{
    m_singleJob = true;
    return startJob( job, jobData, repeat, false );
}

void JobThread::stopJob( JobId id )
{
    m_mutex.lock();
    for ( QList<JobData>::iterator iter = m_jobList.begin(); iter != m_jobList.end(); ++iter )
    {
        if( (*iter).id == id )
        {
            m_jobList.erase( iter );
            break;
        }
    }
    m_mutex.unlock();
}

void JobThread::stop()
{
    m_doJob = false;
    m_mutex.lock();
    m_jobList.clear();
    m_mutex.unlock();
    m_semaphore.release();
    wait();
}

JobThread::JobId JobThread::startJob( JobWorker job, void* jobData, bool mutexlJob, bool repeat )
{
    if( m_singleJob && m_jobList.size() >= 1 )
        return m_jobList.at(0).id;

    if( mutexlJob )
    {
        int jobId = InvalidID;
        m_mutex.lock();
        for ( QList<JobData>::const_iterator iter = m_jobList.constBegin(); iter != m_jobList.constEnd(); ++iter )
        {
            if( (*iter).job == job )
            {
                jobId = (*iter).id;
                break;
            }
        }
        m_mutex.unlock();
        if( jobId != InvalidID )
            return jobId;
    }

    JobId id;
    bool loop = true;
    while ( loop )
    {
        loop = false;
        id = g_jobIdProvider++;
        if( id == InvalidID ) //can not be zero
            id = g_jobIdProvider++;
        foreach( const JobData& job, m_jobList )
        {
            if( job.id == id )
            {
                loop = true;
                break;
            }
        }
    }
    JobData data_p;
    data_p.job = job;
    data_p.data = jobData;
    data_p.isRepeat = repeat;
    data_p.id = id;
    m_mutex.lock();
    m_jobList.push_back( data_p );
    if( !m_doJob )
    {
        m_doJob = true;
        start();
    }
    m_mutex.unlock();
    m_semaphore.release();
    return data_p.id;
}

void JobThread::run()
{
    while ( m_doJob )
    {
        JobData job;

        m_mutex.lock();
        if( !m_jobList.isEmpty() )
        {
            job = m_jobList.front();
            m_jobList.pop_front();
            if( job.isRepeat )
                m_jobList.push_back( job );
        }
        m_mutex.unlock();
        
        if( job.id == InvalidID )
        {
            m_semaphore.acquire();
            m_semaphore.acquire( m_semaphore.available() );
            continue;
        }

        job.job( job.data );
    }
}
