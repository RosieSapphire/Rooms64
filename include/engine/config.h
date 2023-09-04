#ifndef ENGINE_CONFIG_H_
#define ENGINE_CONFIG_H_

#define FRAMERATE 24.0f
#define DELTATIME (1.0f / FRAMERATE)
#define DELTATICKS (DELTATIME * (float)TICKS_PER_SECOND)

#define UNUSED __attribute__((unused))

#endif /* ENGINE_CONFIG_H_ */
