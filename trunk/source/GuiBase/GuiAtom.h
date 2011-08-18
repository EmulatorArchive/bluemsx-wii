#ifndef _GUI_ATOM_H
#define _GUI_ATOM_H

class GuiRootContainer;

class GuiAtom
{
public:
    GuiAtom();
    GuiAtom(GuiRootContainer* root);
    virtual ~GuiAtom();

    virtual void Delete(GuiAtom *atom);

    static GuiRootContainer* GetRootContainer(void);

protected:
    static void SetRootContainer(GuiRootContainer* root);

private:
    static GuiRootContainer* m_poRootContainer;
};

#endif

