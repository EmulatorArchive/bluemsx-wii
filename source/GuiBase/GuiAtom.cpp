/***************************************************************
 *
 * Copyright (C) 2008-2011 Tim Brugman
 *
 * This file may be licensed under the terms of of the
 * GNU General Public License Version 2 (the ``GPL'').
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the GPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the GPL along with this
 * program. If not, go to http://www.gnu.org/licenses/gpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ***************************************************************/

#include <assert.h>

#include "GuiAtom.h"
#include "GuiRootContainer.h"

GuiRootContainer* GuiAtom::m_poRootContainer = NULL;

GuiAtom::GuiAtom(GuiRootContainer* root)
{
    m_poRootContainer = root;
}

GuiAtom::GuiAtom()
{
    if( m_poRootContainer != NULL ) {
        m_poRootContainer->RegisterAtom(this);
    }
}

GuiAtom::~GuiAtom()
{
#ifdef DEBUG
    assert( m_poRootContainer != NULL );
#endif
    m_poRootContainer->AbandonAtom(this);
}

void GuiAtom::SetRootContainer(GuiRootContainer* root)
{
    assert( m_poRootContainer == NULL || root == NULL ); // only one root allowed
    m_poRootContainer = root;
}

GuiRootContainer* GuiAtom::GetRootContainer(void)
{
    return m_poRootContainer;
}

bool GuiAtom::IsAlive(GuiAtom *atom)
{
    assert( m_poRootContainer != NULL );
    return m_poRootContainer->IsAtomRegistered(atom);
}

void GuiAtom::Delete(GuiAtom *atom)
{
    if( m_poRootContainer != NULL ) {
        m_poRootContainer->ReleaseAtom(atom);
    }
}

