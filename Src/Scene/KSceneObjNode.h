/************************************************************************/
/* �������������ڵ�                                                   */
/* Copyright : Kingsoft 2005										    */
/* Author	 : Zhu Jianqiu												*/
/* History	 :															*/
/*		2004.12.31	Create												*/
/************************************************************************/
#ifndef _KSCENE_OBJ_NODE_H_
#define _KSCENE_OBJ_NODE_H_

class KSceneObject;

class KSceneObjNode : public KNode
{
public:
	KSceneObjNode(void);
	~KSceneObjNode(void);

	KSceneObject*	m_pSceneObject;
};

#endif	//_KSCENE_OBJ_NODE_H_
