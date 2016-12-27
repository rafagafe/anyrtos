
/*
 * Developed by Rafa Garcia <rafagarcia77@gmail.com>
 *
 * prompt.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * prompt.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _PROMPT_
#define _PROMPT_

/** @defgroup prompt Prompt
  * @{ */

/** Receives commands by serial port. */
void prompt_task( void* param );

/** @} */

#endif /* _PROMPT_ */
