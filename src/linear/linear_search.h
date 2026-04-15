#ifndef LINEAR_SEARCH_H
#define LINEAR_SEARCH_H

#include "../common/player.h"

/* id가 일치하는 첫 번째 레코드 반환. 없으면 NULL */
Player *linear_search(Player *table, int size, int target_id);

/* id가 lo 이상 hi 이하인 레코드 개수 반환 */
int linear_range(Player *table, int size, int lo, int hi);

#endif
