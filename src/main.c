#include "networking/discover.h"

# define PORT 5125
# define POLL_MS 2000

int main(void) {
	discovery(PORT,POLL_MS, uid());
	return 0;
}
