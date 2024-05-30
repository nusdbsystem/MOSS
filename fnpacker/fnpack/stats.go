package fnpack

import (
	"sync"
	"time"
)

// Model statistics
type ModelStats interface {
	UpdateLastCall()
	UpdateStats(start_time, end_time time.Time, exec uint64)
	GenEstNext() int64
	GenTimeSinceLastCall() int64
	GenSwitchCost() int64
	GenCost() int64
}

// launch time can be conservatively estimated to be max_exec-min_exec
type DefaultModelStats struct {
	last_call     int64
	interarrival  int64
	exec_time     int64
	min_exec_time int64
	max_exec_time int64
	mu            sync.RWMutex
}

func (ms *DefaultModelStats) UpdateLastCall() {
	ms.mu.Lock()
	defer ms.mu.Unlock()
	ms.last_call = time.Now().UnixMicro()
}

func (ms *DefaultModelStats) UpdateStats(st, et time.Time, exec uint64) {
	ms.mu.Lock()
	ms.exec_time = int64(exec)
	// conservative last invocation finish time
	if st.UnixMicro() > ms.last_call {
		// first invocation, conservatively left interarrival to 0
		if ms.last_call != 0 {
			ms.interarrival = st.UnixMicro() - ms.last_call
		}
		ms.last_call = et.UnixMicro()
	} else {
		ms.interarrival = 0
	}
	ms.mu.Unlock()
}

// not used
func (ms *DefaultModelStats) GenEstNext() int64 {
	ms.mu.RLock()
	defer ms.mu.RUnlock()
	return ms.last_call + ms.interarrival
}

func (ms *DefaultModelStats) GenTimeSinceLastCall() int64 {
	ms.mu.RLock()
	defer ms.mu.RUnlock()
	return time.Now().UnixMicro() - ms.last_call
}

// not used
func (ms *DefaultModelStats) GenSwitchCost() int64 {
	ms.mu.RLock()
	defer ms.mu.RUnlock()
	// clip the cost to be 10 * exec time
	launch_est := ms.max_exec_time - ms.min_exec_time
	if launch_est > 10*ms.exec_time {
		return 10 * ms.exec_time
	} else {
		return launch_est
	}
}

func (ms *DefaultModelStats) GenCost() int64 {
	ms.mu.RLock()
	defer ms.mu.RUnlock()
	return ms.exec_time
}
