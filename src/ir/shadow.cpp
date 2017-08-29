//
// Created by GentlyGuitar on 6/6/2017.
//

#include <utilities/strings.h>
#include "shadow.h"
#include "../utilities/flags.h"

/**@brief
 *
 * Caution: Currently this function will does a simple replacement of
 * the old value by the new value. If for some reason there are multiple
 * occurences of the old value string in the text, some unwanted replacement
 * could happen
 *
 * @param field
 * @param new_value
 */
void Shadow::update_raw_field(string field, string new_value) {
    if (has_raw_field(field)) {
        string old_value = get_raw_field(field);
        Strings::replace(_raw_text, old_value, new_value);
        set_raw_field(field, new_value);
    }
}