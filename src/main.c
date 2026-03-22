#include "networking/discover.h"

# define PORT 5125
# define POLL_MS 2000
# define TCP_PORT 1226

int main(void) {
	discovery(PORT,TCP_PORT,POLL_MS, uid());
	return 0;
}
