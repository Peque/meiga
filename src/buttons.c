#include "buttons.h"

/**
 * @brief Function to read user button.
 */
bool read_user_button(void)
{
	return (bool)(gpio_get(GPIOC, GPIO13));
}
