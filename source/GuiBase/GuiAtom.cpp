
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
    assert( !m_poRootContainer->IsAtomRegistered(this) );
#endif
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

void GuiAtom::Delete(GuiAtom *atom)
{
    if( m_poRootContainer != NULL ) {
        m_poRootContainer->ReleaseAtom(atom);
    }
}

