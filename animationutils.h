#ifndef ANIMATIONUTILS_H
#define ANIMATIONUTILS_H

namespace AnimationUtils
{

template<class T>
T animate(T currentPosition, T targetPosition, T step, bool& needAnotherStep)
{
	T result = currentPosition;
	if(result < targetPosition)
	{
		result += step;
		if(result > targetPosition)
			result = targetPosition;
		else
			needAnotherStep = true;
	}
	else
	{
		result -= step;
		if(result < targetPosition)
			result = targetPosition;
		else
			needAnotherStep = true;
	}
	return result;
}

}

#endif
