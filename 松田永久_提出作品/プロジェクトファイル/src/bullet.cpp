//=============================================
//
//弾[bullet.cpp]
//Author Matsuda Towa
//
//=============================================
#include "bullet.h"
#include "manager.h"
#include "game.h"
#include "active_player.h"

//テクスチャパス
const std::string CBullet::BULLET_TEXTURE_NAME ="data\\TEXTURE\\bullet.png";

//=============================================
//コンストラクタ
//=============================================
CBullet::CBullet(int nPriority):CAttack_Manager(nPriority)
{
}

//=============================================
//デストラクタ
//=============================================
CBullet::~CBullet()
{
}

//=============================================
//初期化
//=============================================
HRESULT CBullet::Init()
{
	//親クラスの初期化
	CAttack_Manager::Init();

	return S_OK;
}

//=============================================
//終了
//=============================================
void CBullet::Uninit()
{
	//親クラスの終了
	CAttack_Manager::Uninit();
}

//=============================================
//更新
//=============================================
void CBullet::Update()
{
	//親クラスの更新
	CAttack_Manager::Update();
	
	//使用しているとき
	OnActive();

}

//=============================================  
//描画
//=============================================
void CBullet::Draw()
{
	//親クラスの描画
	CAttack_Manager::Draw();
}

//=============================================
//使用されてる時の処理
//=============================================
void CBullet::OnActive()
{
	//体力取得
	int nLife = GetLife();

	if (nLife <= 0)
	{
		//終了
		Uninit();
		return;
	}

	//ライフがあれば処理実行
	D3DXVECTOR3 pos = GetPos();
	pos += m_move;
	//座標を更新
	SetPos(pos);
	//頂点座標
	SetVtx(D3DXVECTOR3(0.0f, 0.0f, -1.0f));

	//当たり判定チェック変数
	bool bHitCheck = false;

	switch (m_Allegiance)
	{
	case BULLET_ALLEGIANCE_PLAYER:
		bHitCheck = HitEnemy();
		if (bHitCheck)
		{
			SetHitMaker();
		}
		break;

	case BULLET_ALLEGIANCE_ENEMY:
		bHitCheck = HitPlayer();
		break;

	default:
		assert(false);
		break;
	}

	//当たり判定チェックTODO:レイでやれ
	bHitCheck = HitBlock();

	//当たり判定チェック変数
	bool bPenetration = false;

	bPenetration = PenetrationBlock();

	if (bHitCheck == true || bPenetration == true)
	{//当たってたら
		Uninit();
	}
}

//=============================================
//ヒットマーカー作成
//=============================================
void CBullet::SetHitMaker()
{
	for (int nCnt = 0; nCnt < MAX_OBJECT; nCnt++)
	{
		//オブジェクト取得
		CObject* pObj = CObject::Getobject(CPlayer::PLAYER_PRIORITY, nCnt);
		if (pObj == nullptr)
		{//ヌルポインタなら
			//オブジェクトを探し続ける
			continue;
		}
		 //タイプ取得
		CObject::OBJECT_TYPE type = pObj->GetType();

		//プレイヤーを探す
		if (type != CObject::OBJECT_TYPE::OBJECT_TYPE_PLAYER)
		{//プレイヤーじゃなければ
			//プレイヤーを探し続ける
			continue;
		}

		CActivePlayer* pPlayer = dynamic_cast<CActivePlayer*>(pObj);

		if (pPlayer->GetReticle() != nullptr)
		{
			if (pPlayer->GetReticle()->m_pHitMaker != nullptr)
			{
				pPlayer->GetReticle()->m_pHitMaker->SetColor(COLOR_WHITE);
			}
		}
	}
}

//=============================================
//弾作成
//=============================================
CBullet* CBullet::Create(D3DXVECTOR3 pos, D3DXVECTOR3 move, D3DXVECTOR3 rot, D3DXVECTOR3 size
,int nLife, int nDamage, BULLET_ALLEGIANCE Allegiance)
{
	CBullet* pBullet = nullptr;

	pBullet = new CBullet;

	if (pBullet == nullptr) {return nullptr;}

	CTexture* pTexture = CManager::GetInstance()->GetTexture();

	pBullet->SetPos(pos); //pos設定
	pBullet->SetSize(size); //サイズ設定
	pBullet->SetRot(rot);	//方向設定
	pBullet->m_move = move; //移動量代入
	pBullet->SetLife(nLife); //寿命代入
	pBullet->SetDamage(nDamage); //威力代入

	pBullet->BindTexture(pTexture->GetAddress(pTexture->Regist(&BULLET_TEXTURE_NAME)));

	pBullet->m_Allegiance = Allegiance; //弾の設定
	pBullet->SetType(OBJECT_TYPE_BULLET); //タイプ設定
	pBullet->Init();

	return pBullet;
}

//=============================================
//ブロックに当たったかどうか
//=============================================
bool CBullet::PenetrationBlock()
{
	//位置取得
	D3DXVECTOR3 Attackpos = GetPos();
	//サイズ取得
	D3DXVECTOR3 Attacksize = GetSize();

	for (int nCnt = 0; nCnt < MAX_OBJECT; nCnt++)
	{
		//オブジェクト取得
		CObject* pObj = CObject::Getobject(CBlock::BLOCK_PRIORITY, nCnt);
		if (pObj == nullptr)
		{//ヌルポインタなら
			//オブジェクトを探し続ける
			continue;
		}
		//タイプ取得
		CObject::OBJECT_TYPE type = pObj->GetType();

		//ブロックを探す
		if (type != CObject::OBJECT_TYPE::OBJECT_TYPE_BLOCK)
		{//ブロックじゃなければ
			//ブロックを探し続ける
			continue;
		}

		//ブロックとの当たり判定
		CBlock* pBlock = dynamic_cast<CBlock*>(pObj);

		CColision::COLISION ColisionCheck_X = CManager::GetInstance()->GetColision()->CheckPolygonModelPenetration_X(Attackpos - m_move, Attackpos, pBlock->GetPos(), pBlock->GetMinPos(), pBlock->GetMaxPos());
		CColision::COLISION ColisionCheck_Z = CManager::GetInstance()->GetColision()->CheckPolygonModelPenetration_Z(Attackpos - m_move, Attackpos, pBlock->GetPos(), pBlock->GetMinPos(), pBlock->GetMaxPos());

		if (ColisionCheck_X != CColision::COLISION::COLISON_NONE || ColisionCheck_Z != CColision::COLISION::COLISON_NONE)
		{//当たってたら
			//攻撃の削除
			return true;
		}
	}
	return false;
}
