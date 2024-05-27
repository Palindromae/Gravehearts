#pragma once
enum class ThreadStatus {
	Starting,
	Ready,
	Working,
	Processing,
	Processed,
	Awaiting,
	Exiting
};