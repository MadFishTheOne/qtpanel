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

template<class T>
T animateExponentially(T currentPosition, T targetPosition, qreal factor, T minimalStep, bool& needAnotherStep)
{
	T step = static_cast<T>(abs(targetPosition - currentPosition)*factor);
	if(step < minimalStep)
		step = minimalStep;
	return animate(currentPosition, targetPosition, step, needAnotherStep);
}

}

#endif
