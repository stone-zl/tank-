#include "stdafx.h"
#include "PlayerBase.h"

//----------------- PlayerBase 类静态数据

int PlayerBase::mDevXY[4][2] = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} };	// 依次左上右下

PlayerBase::PlayerBase(byte player, BoxMarkStruct* b)
{
	mDied = false;
	int i = 0;
	player_id = player;
	mPlayerTank = new PlayerTank(player_id);
	bms = b;

	// 不同玩家数据不同
	if (player_id == 0)
	{
		loadimage(&m12PImage, _T("./res/big/1P.gif"));		// 1P\2P图标
		m12PImage_x = 233;									// 1P\2P 坐标
		m12PImage_y = 129;
		mPlayerTankIcoImage_x = 232;						// 玩家坦克图标坐标
		mPlayerTankIcoImage_y = 137;
		mPlayerLife_x = 240;								// 玩家生命值坐标
		mPlayerLife_y = 137;
		mTankX = 4 * 16 + BOX_SIZE;							// 坦克首次出现时候的中心坐标
		mTankY = 12 * 16 + BOX_SIZE;
	}
	else
	{
		loadimage(&m12PImage, _T("./res/big/2P.gif"));
		m12PImage_x = 233;
		m12PImage_y = 153;
		mPlayerTankIcoImage_x = 232;
		mPlayerTankIcoImage_y = 161;
		mPlayerLife_x = 240;
		mPlayerLife_y = 161;
		mTankX = 8 * 16 + BOX_SIZE;
		mTankY = 12 * 16 + BOX_SIZE;
	}

	// 共同的数据
	loadimage(&mPlayerTankIcoImage, _T("./res/big/playertank-ico.gif"	));	// 玩家坦克图标
	loadimage(&mBlackNumberImage,	_T("./res/big/black-number.gif"		));	// 黑色数字
	mPlayerLife = 2;		// 玩家 HP
	mPlayerTankLevel = 0;													// 坦克级别 [0-3]
	mTankDir = DIR_UP;		// 坦克方向

	// 不同级别坦克移动速度系数
	int temp[4] = {2, 2, 3, 3};
	for ( i = 0; i < 4; i++ )
		mSpeed[i] = temp[i];

	/*********************************
	* BulletStruct 数据初始化
	*********************************/
	// 加载子弹图片
	TCHAR buf[100];
	for (i = 0; i < 4; i++)
	{
		_stprintf_s(buf, _T("./res/big/bullet-%d.gif"), i);
		loadimage(&BulletStruct::mBulletImage[i], buf);
	}

	// 子弹结构数据
	int temp_speed[4] = {3, 3, 4, 4};			// 根据坦克级别分配子弹速度系数
	for (i = 0; i < 2; i++)
	{
		mBulletStruct[i].x = SHOOTABLE_X;		// x 坐标用于判断是否可以发射
		mBulletStruct[i].y = -1000;
		mBulletStruct[i].dir = DIR_UP;
		mBulletStruct[i].mKillId = 0;			// 记录击中的敌机 id

		// 根据坦克级别分配子弹速度系数
		for (int j = 0; j < 4; j++)
			mBulletStruct[i].speed[j] = temp_speed[j];
	}

	mBullet_1_counter = 9;				// 子弹 1 运动 N 个循环后才可以发射子弹 2 
	mMoving = false;

	// 爆炸图片
	for (i = 0; i < 3; i++)
	{
		_stprintf_s(buf, _T("./res/big/bumb%d.gif"), i);
		loadimage(&BombStruct::mBombImage[i], buf);
	}
	for (i = 0; i < 2; i++)
	{
		mBombS[i].mBombX = -100;
		mBombS[i].mBombY = -100;
		mBombS[i].canBomb = false;
		mBombS[i].counter = 0;
	}

	// 坦克爆炸图片结构
	for (i = 0; i < 5; i++)
	{
		_stprintf_s(buf, _T("./res/big/blast/%d.gif"), i);
		loadimage(&BlastStruct::image[i], buf);
	}

	// 是否击中大本营
	mIsShootCamp = false;

	// 出生四角星闪烁
	for (int i = 0; i < 4; i++)
	{
		_stprintf_s(buf, _T("./res/big/star%d.gif"), i);
		loadimage(&StarClass::mStarImage[i], buf);
	}

	// 出生保护环
	for (int i = 0; i < 2; i++)
	{
		_stprintf_s(buf, _T("./res/big/ring%d.gif"), i);
		loadimage(&RingClass::image[i], buf);
	}
}

PlayerBase::~PlayerBase()
{
}

// 绘制玩家的一些数据: 1P\2P 坦克图标 生命
void PlayerBase::DrawPlayerTankIco(const HDC& right_panel_hdc)
{
	// 绘制1P/2P
	TransparentBlt(right_panel_hdc, m12PImage_x, m12PImage_y, PLAYER_12_ICO_SIZE_X, PLAYER_12_ICO_SIZE_Y,
							GetImageHDC(&m12PImage), 0, 0, PLAYER_12_ICO_SIZE_X, PLAYER_12_ICO_SIZE_Y, 0xffffff);
	// 绘制坦克图标
	TransparentBlt(right_panel_hdc, mPlayerTankIcoImage_x, mPlayerTankIcoImage_y, PLAYER_TANK_ICO_SIZE_X, PLAYER_TANK_ICO_SIZE_Y,
									GetImageHDC(&mPlayerTankIcoImage), 0, 0, PLAYER_TANK_ICO_SIZE_X, PLAYER_TANK_ICO_SIZE_Y, 0x000000);
	// 绘制玩家生命数字
	TransparentBlt(right_panel_hdc, mPlayerLife_x, mPlayerLife_y, BLACK_NUMBER_SIZE, BLACK_NUMBER_SIZE,
		GetImageHDC(&mBlackNumberImage), BLACK_NUMBER_SIZE * mPlayerLife, 0, BLACK_NUMBER_SIZE, BLACK_NUMBER_SIZE, 0xffffff );
}

bool PlayerBase::ShowStar(const HDC& center_hdc)
{
	// 坦克已经出现,不用闪烁,直接返回
	if (mStar.mIsOuted == true)
		return STOP_SHOW_STAR;

	// 开始闪烁四角星
	if (mStar.mStarCounter++ % 2 == 0)
	{
		if (mStar.mStarIndex + mStar.mStarIndexDev < 0)
		{
			mStar.mStarIndex = 1;
			mStar.mStarIndexDev = 1;
		}
		else if (mStar.mStarIndex + mStar.mStarIndexDev > 3)
		{
			mStar.mStarIndex = 2;
			mStar.mStarIndexDev = -1;
		}
		else
		{
			mStar.mStarIndex += mStar.mStarIndexDev;
		}
		if (mStar.mStarCounter == 25)
		{
			mStar.mIsOuted = true;						// 结束闪烁, TankMoving() 函数开始循环, 坦克开始移动
			SignBox_8(mTankX, mTankY, PLAYER_SIGN + player_id);		// 坦克出现, 将四角星标记改为坦克标记
			return STOP_SHOW_STAR;
		}
	}

	TransparentBlt(center_hdc, (int)mTankX - BOX_SIZE, (int)mTankY - BOX_SIZE, BOX_SIZE * 2, BOX_SIZE * 2,
		GetImageHDC(&StarClass::mStarImage[mStar.mStarIndex]), 0, 0, BOX_SIZE * 2, BOX_SIZE * 2, 0x000000);

	return SHOWING_STAR;
}

//
void PlayerBase::DrawPlayerTank(const HDC& canvas_hdc)
{
	if (!mStar.mIsOuted)
		return;

	IMAGE tank = mPlayerTank->GetTankImage(mPlayerTankLevel, mTankDir, mMoving);
	TransparentBlt(canvas_hdc, (int)(mTankX - BOX_SIZE), (int)(mTankY - BOX_SIZE), BOX_SIZE * 2, BOX_SIZE * 2, GetImageHDC(&tank), 0, 0, BOX_SIZE * 2, BOX_SIZE * 2, 0x000000);

	if (mRing.canshow)
		mRing.ShowRing(canvas_hdc, mTankX, mTankY);
}

//
bool PlayerBase::PlayerControl()
{
	if (mDied)
		return true;

	switch (player_id)
	{
	case 0:										// 玩家一
		if (GetAsyncKeyState('A') & 0x8000)
		{
			mMoving = true;
			Move(DIR_LEFT);
		}
		else if (GetAsyncKeyState('W') & 0x8000)
		{
			mMoving = true;
			Move(DIR_UP);
		}
		else if (GetAsyncKeyState('D') & 0x8000)
		{
			mMoving = true;
			Move(DIR_RIGHT);
		}
		else if (GetAsyncKeyState('S') & 0x8000)
		{
			mMoving = true;
			Move(DIR_DOWN);
		}
		else
		{
			mMoving = false;
		}

		// 不能加 else if, 不然移动时候无法发射子弹
		if (GetAsyncKeyState('J') & 0x8000)	// 发射子弹
		{
			if (!ShootBullet(0))
				ShootBullet(1);
		}
		break;

	case 1:										// 玩家二
		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			mMoving = true;
			Move(DIR_LEFT);
		}
		else if (GetAsyncKeyState(VK_UP) & 0x8000)
		{
			mMoving = true;
			Move(DIR_UP);
		}
		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			mMoving = true;
			Move(DIR_RIGHT);
		}
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			mMoving = true;
			Move(DIR_DOWN);
		}else
		{
			mMoving = false;
		}

		// 数字键 1 发射子弹
		if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
		{
			if (!ShootBullet(0))
				ShootBullet(1);
		}
		break;
	default:
		break;
	}
	return true;
}

//
void PlayerBase::BulletMoving(const HDC& center_hdc)
{
	if (mDied)
		return;

	for (int i = 0; i < 2; i++)
	{
		// 子弹在移动
		if (mBulletStruct[i].x != SHOOTABLE_X)
		{
			// 检测打中障碍物与否
			if (CheckBomb(i))
				continue;

			int dir = mBulletStruct[i].dir;
			mBulletStruct[i].x += mDevXY[dir][0] * mBulletStruct[i].speed[mPlayerTankLevel];
			mBulletStruct[i].y += mDevXY[dir][1] * mBulletStruct[i].speed[mPlayerTankLevel];

			TransparentBlt(center_hdc, mBulletStruct[i].x, mBulletStruct[i].y, BulletStruct::mBulletSize[dir][0],
				BulletStruct::mBulletSize[dir][1], GetImageHDC(&BulletStruct::mBulletImage[dir]),
				0, 0, BulletStruct::mBulletSize[dir][0], BulletStruct::mBulletSize[dir][1], 0x000000);

			// 记录子弹 1 的步数, 决定可否发射子弹 2
			if ( i == 0 )
				mBullet_1_counter--;
		}
	}
	/*
	if (mPlayerTankLevel > 1 && mBulletX[1] != SHOOTABLE_X)
	{
		mBulletX[1] += mDevXY[mBulletDir[1]][0] * 6;
		mBulletY[1] += mDevXY[mBulletDir[1]][1] * 6;
	}*/
}


void PlayerBase::Bombing(const HDC& center_hdc)
{
	int index[3] = {0,1,2};
	for (int i = 0; i < 2; i++)
	{
		if (mBombS[i].canBomb)
		{
			TransparentBlt(center_hdc, mBombS[i].mBombX - BOX_SIZE, mBombS[i].mBombY - BOX_SIZE, BOX_SIZE * 2, BOX_SIZE * 2,
				GetImageHDC(&BombStruct::mBombImage[index[mBombS[i].counter % 3]]), 0, 0, BOX_SIZE * 2, BOX_SIZE * 2, 0x000000);
			if (mBombS[i].counter++ == 3)
				mBombS[i].canBomb = false;
		}
	}
}

// GameControl 内调用, 通过参数将 mBulletStruct.mKillId 传递进去
void PlayerBase::GetKillEnemy(int& bullet1, int& bullet2)
{
	bullet1 = mBulletStruct[0].mKillId;
	bullet2 = mBulletStruct[1].mKillId;
	
	// 重置标志
	mBulletStruct[0].mKillId = 0;
	mBulletStruct[1].mKillId = 0;
}

bool PlayerBase::IsShootCamp()
{
	return mIsShootCamp;
}

void PlayerBase::BeKill()
{
	mDied = true;
	SignBox_8(mTankX, mTankY, _EMPTY);

	// 设置爆炸坐标
	mBlast.blastx = mTankX;
	mBlast.blasty = mTankY;
	mBlast.canBlast = true;
}

// 玩家被击中爆炸
bool PlayerBase::Blasting(const HDC & center_hdc)
{
	int index[6] = { 0,1,2,3,4,2 };
	if (mBlast.canBlast)
	{
		TransparentBlt(center_hdc, mBlast.blastx - BOX_SIZE * 2, mBlast.blasty - BOX_SIZE * 2, BOX_SIZE * 4, BOX_SIZE * 4,
			GetImageHDC(&BlastStruct::image[index[mBlast.counter % 6]]), 0, 0, BOX_SIZE * 4, BOX_SIZE * 4, 0x000000);
		if (mBlast.counter++ == 6)
		{
			mBlast.canBlast = false;
			return true;
		}
	}
	return false;
}

//
int PlayerBase::GetID()
{
	return player_id;
}

//---------------------------------------------------------------- private function ---------

// 变向的同时调整坦克所在格子. 必须保证坦克中心在格子线上
void PlayerBase::Move(int new_dir)
{
	SignBox_8(mTankX, mTankY, _EMPTY);

	if (mTankDir != new_dir)
	{
		// 原左右变上下方向
		if (mTankDir == DIR_LEFT || mTankDir == DIR_RIGHT)
		{
			if (mTankX > (mTankX / BOX_SIZE) * BOX_SIZE + BOX_SIZE / 2 - 1)	// 如果是靠近格子线上的右边节点, -1是修正
				mTankX = (mTankX / BOX_SIZE + 1) * BOX_SIZE;
			else
				mTankX = (mTankX / BOX_SIZE) * BOX_SIZE;					// 靠近格子线上的左边节点
		}
		// 上下变左右
		else
		{
			if (mTankY > (mTankY / BOX_SIZE) * BOX_SIZE + BOX_SIZE / 2 - 1)	// 如果是靠近格子线上的下边节点, -1是修正
				mTankY = (mTankY / BOX_SIZE + 1) * BOX_SIZE;
			else
				mTankY = (mTankY / BOX_SIZE) * BOX_SIZE;					// 靠近格子线上的上边节点
		}

		// 更改方向, 必须先调正位置后才能设置方向!!
		mTankDir = new_dir;
	}
	else								// 移动
	{
		if (CheckMoveable())
		{
			mTankX += mDevXY[mTankDir][0] * mSpeed[mPlayerTankLevel];
			mTankY += mDevXY[mTankDir][1] * mSpeed[mPlayerTankLevel];
		}
	}
	SignBox_8(mTankX, mTankY, PLAYER_SIGN + player_id);
}

/* 判断当前方向可否移动
	box_8[i][j]
	i = y / BOX_SIZE		// y 坐标对应的是 i 值
	j = x / BOX_SIZE		// x 坐标对应的是 j 值
	---------
	| 1 | 2	|
	----a----
	| 3 | 4	|
	---------
* 如果 (x,y) 在 a 点出, 转换后的 i,j 属于格子 4
* 如果 x 值在 a 点左边, 则转换后的 j 属于 1或3; 右边则属于 2或4
* 如果 y 值在 a 点以上, 则转换后的 i 属于 1或2; 以下则属于 3或4
** 如果 tempx,tempy 跨越了格子又遇到障碍, 那么就将 mTankX 或 mTankY 调整到格子线上,
*/
bool PlayerBase::CheckMoveable()
{
	// 坦克中心坐标
	int tempx = mTankX + mDevXY[mTankDir][0] * mSpeed[mPlayerTankLevel];
	int tempy = mTankY + mDevXY[mTankDir][1] * mSpeed[mPlayerTankLevel];

	if (tempx < BOX_SIZE || tempy < BOX_SIZE || tempy > CENTER_WIDTH - BOX_SIZE || tempx > CENTER_HEIGHT - BOX_SIZE)
	{
		// 如果遇到障碍物,将坦克坐标调整到格子线上. 不然坦克和障碍物会有几个像素点间隔
		switch (mTankDir)
		{
		case DIR_LEFT:	mTankX = (mTankX / BOX_SIZE) * BOX_SIZE;	break;	// mTankX 与 tempx 之间跨越了格子, 将坦克放到mTankX所在的格子线上
		case DIR_UP:	mTankY = (mTankY / BOX_SIZE) * BOX_SIZE;	break;
		case DIR_RIGHT: mTankX = (tempx  / BOX_SIZE) * BOX_SIZE;	break;
		case DIR_DOWN:	mTankY = (tempy  / BOX_SIZE) * BOX_SIZE;	break;
		default:													break;
		}
		return false;
	}
	// 转换像素点所在的 xy[26][26] 下标
	int index_i = (int)tempy / BOX_SIZE;
	int index_j = (int)tempx / BOX_SIZE;

	int dev[4][2][2] = { {{-1,-1},{0,-1}},  {{-1,-1},{-1,0}},  {{-1,1},{0,1}}, { {1,-1},{1,0}} };

	if (bms->box_8[index_i + dev[mTankDir][0][0]][index_j + dev[mTankDir][0][1]] > 2 ||
		bms->box_8[index_i + dev[mTankDir][1][0]][index_j + dev[mTankDir][1][1]] > 2 )
	{
		// 如果遇到障碍物,将坦克坐标调整到格子线上. 不然坦克和障碍物会有几个像素点间隔
		switch (mTankDir)
		{
		case DIR_LEFT:	mTankX = (mTankX / BOX_SIZE) * BOX_SIZE;	break;	// mTankX 与 tempx 之间跨越了格子, 将坦克放到mTankX所在的格子线上
		case DIR_UP:	mTankY = (mTankY / BOX_SIZE) * BOX_SIZE;	break;
		case DIR_RIGHT: mTankX = (tempx  / BOX_SIZE) * BOX_SIZE;	break;
		case DIR_DOWN:	mTankY = (tempy  / BOX_SIZE) * BOX_SIZE;	break;
		default:													break;
		}
		return false;
	}
	return true;
}

// 发射子弹
bool PlayerBase::ShootBullet( int bullet_id )
{
	switch (bullet_id)
	{
		case 0:
			// 1号子弹发射失败, 爆炸未完成前不能发射
			if (mBulletStruct[0].x != SHOOTABLE_X || mBombS[0].canBomb == true)		
				return false;

			// 子弹发射点坐标
			mBulletStruct[0].x = (int)(mTankX + BulletStruct::devto_tank[mTankDir][0]);
			mBulletStruct[0].y = (int)(mTankY + BulletStruct::devto_tank[mTankDir][1]);
			mBulletStruct[0].dir = mTankDir;
			mBullet_1_counter = 6;
			return true;

		case 1:
			// 2 号子弹发射失败
			if (mPlayerTankLevel < 2 || mBulletStruct[1].x != SHOOTABLE_X || mBullet_1_counter > 0 || mBombS[1].canBomb == true)
				return false;

			// 子弹发射点坐标
			mBulletStruct[1].x = (int)(mTankX + BulletStruct::devto_tank[mTankDir][0]);
			mBulletStruct[1].y = (int)(mTankY + BulletStruct::devto_tank[mTankDir][1]);
			mBulletStruct[1].dir = mTankDir;
			return true;

		default:
			break;
	}
	return false;
}

//
bool PlayerBase::CheckBomb(int i)
{
	//if (mBombS[i].canBomb)
	//	return true;
	int dir = mBulletStruct[i].dir;

	// 子弹头接触到障碍物的那个点, 左右方向点在上, 上下方向点在右
	int bombx = mBulletStruct[i].x + BulletStruct::devto_head[dir][0];
	int bomby = mBulletStruct[i].y + BulletStruct::devto_head[dir][1];

	bool flag = false;
	int adjust_x = 0, adjust_y = 0;		// 修正爆照图片显示的坐标

	// 不能用 bombx 代替 mBulletStruct[i].x,否则会覆盖障碍物的检测
	if (mBulletStruct[i].x < 0 && mBulletStruct[i].dir == DIR_LEFT)
	{
		flag = true;
		adjust_x = 5;					// 将爆炸图片向右移一点
	}
	else if (mBulletStruct[i].y < 0 && mBulletStruct[i].dir == DIR_UP)
	{
		flag = true;
		adjust_y = 5;
	}

	// 必须减去子弹的宽 4, 不然子弹越界, 后面检测导致 box_8 下标越界
	else if (mBulletStruct[i].x >= CENTER_WIDTH - 4 && mBulletStruct[i].dir == DIR_RIGHT)
	{
		flag = true;
		adjust_x = -4;
	}
	else if (mBulletStruct[i].y >= CENTER_HEIGHT - 4 && mBulletStruct[i].dir == DIR_DOWN)
	{
		flag = true;
		adjust_y = -4;
	}
	if (flag)
	{
		// 设定爆炸参数, 修正爆炸中心所在的格子,左右或上下偏移一个格子之类的..
		mBulletStruct[i].x = SHOOTABLE_X;
		mBombS[i].canBomb = true;
		mBombS[i].mBombX = (bombx / SMALL_BOX_SIZE + BulletStruct::bomb_center_dev[mBulletStruct[i].dir][0]) * SMALL_BOX_SIZE;
		mBombS[i].mBombY = (bomby / SMALL_BOX_SIZE + BulletStruct::bomb_center_dev[mBulletStruct[i].dir][1]) * SMALL_BOX_SIZE;
		mBombS[i].counter = 0;

		return true;
	}

	int tempi, tempj;

	// 将坐标转换成 4*4 格子索引
	int bi = bomby / SMALL_BOX_SIZE;
	int bj = bombx / SMALL_BOX_SIZE;

	// 坐标所在 8*8 格子的索引
	int b8i = bomby / BOX_SIZE;
	int b8j = bombx / BOX_SIZE;

	switch (mBulletStruct[i].dir)
	{
	// 左右检测子弹头所在的4*4格子和它上面相邻的那个
	case DIR_LEFT:
	case DIR_RIGHT:
	{
		int temp[2][2] = { {0, 0}, {-1, 0} };
		for (int n = 0; n < 2; n++)
		{
			// 8*8 格子, 判断是否击中敌机
			tempi = b8i + temp[n][0];
			tempj = b8j + temp[n][1];
			if (bms->box_8[tempi][tempj] >= ENEMY_SIGN && bms->box_8[tempi][tempj] < ENEMY_SIGN + TOTAL_ENEMY_NUMBER)
			{
				mBulletStruct[i].x = SHOOTABLE_X;
				mBombS[i].canBomb = true;				// 指示 i bomb 爆炸
				mBombS[i].mBombX = (bombx / SMALL_BOX_SIZE + BulletStruct::bomb_center_dev[mBulletStruct[i].dir][0]) * SMALL_BOX_SIZE;
				mBombS[i].mBombY = (bomby / SMALL_BOX_SIZE + BulletStruct::bomb_center_dev[mBulletStruct[i].dir][1]) * SMALL_BOX_SIZE;
				mBombS[i].counter = 0;

				// 标记击中了敌机的 id
				mBulletStruct[i].mKillId = bms->box_8[tempi][tempj];
				return true;
			}
			else if (bms->box_8[tempi][tempj] == CAMP_SIGN)
			{
				mBulletStruct[i].x = SHOOTABLE_X;
				mBombS[i].counter = 0;
				mIsShootCamp = true;
				SignBox_8(13 * BOX_SIZE, 25 * BOX_SIZE, _EMPTY);
				return true;
			}

			// 检测 4*4 格子, 由此判断障碍物
			tempi = bi + temp[n][0];
			tempj = bj + temp[n][1];
			if (bms->box_4[tempi][tempj] > 2)
			{
				// 设定爆炸参数, 修正爆炸中心所在的格子,左右或上下偏移一个格子之类的..
				mBulletStruct[i].x = SHOOTABLE_X;
				mBombS[i].canBomb = true;				// 指示 i bomb 爆炸
				mBombS[i].mBombX = (bombx / SMALL_BOX_SIZE + BulletStruct::bomb_center_dev[mBulletStruct[i].dir][0]) * SMALL_BOX_SIZE;
				mBombS[i].mBombY = (bomby / SMALL_BOX_SIZE + BulletStruct::bomb_center_dev[mBulletStruct[i].dir][1]) * SMALL_BOX_SIZE;
				mBombS[i].counter = 0;
				ClearWallOrStone(i, bombx, bomby);
				return true;
			}
		}
	}
	break;

	// 上下只检测左右相邻的两个格子
	case DIR_UP:
	case DIR_DOWN:
	{
		int temp[2][2] = { { 0, 0 },{ 0, -1 } };
		for (int n = 0; n < 2; n++)
		{
			// 8*8 格子, 判断是否击中敌机
			tempi = b8i + temp[n][0];
			tempj = b8j + temp[n][1];
			if (bms->box_8[tempi][tempj] >= ENEMY_SIGN && bms->box_8[tempi][tempj] < ENEMY_SIGN + TOTAL_ENEMY_NUMBER)
			{
			//	printf("%d\n", bms->box_8[tempi][tempj]);
				mBulletStruct[i].x = SHOOTABLE_X;
				mBombS[i].canBomb = true;				// 指示 i bomb 爆炸
				mBombS[i].mBombX = (bombx / SMALL_BOX_SIZE + BulletStruct::bomb_center_dev[mBulletStruct[i].dir][0]) * SMALL_BOX_SIZE;
				mBombS[i].mBombY = (bomby / SMALL_BOX_SIZE + BulletStruct::bomb_center_dev[mBulletStruct[i].dir][1]) * SMALL_BOX_SIZE;
				mBombS[i].counter = 0;

				// 标记击中了敌机的 id
				mBulletStruct[i].mKillId = bms->box_8[tempi][tempj];
				return true;
			}
			else if (bms->box_8[tempi][tempj] == CAMP_SIGN)
			{
				mBulletStruct[i].x = SHOOTABLE_X;
				mBombS[i].counter = 0;
				mIsShootCamp = true;
				SignBox_8(13 * BOX_SIZE, 25 * BOX_SIZE, _EMPTY);
				return true;
			}

			// 检测 4*4 是否击中障碍
			tempi = bi + temp[n][0];
			tempj = bj + temp[n][1];
			if (bms->box_4[tempi][tempj] > 2)
			{
				// 设定爆炸参数, 修正爆炸中心所在的格子,左右或上下偏移一个格子之类的..
				mBulletStruct[i].x = SHOOTABLE_X;
				mBombS[i].canBomb = true;				// 指示 i bomb 爆炸
				mBombS[i].mBombX =( bombx / SMALL_BOX_SIZE + BulletStruct::bomb_center_dev[mBulletStruct[i].dir][0]) * SMALL_BOX_SIZE;
				mBombS[i].mBombY =( bomby / SMALL_BOX_SIZE + BulletStruct::bomb_center_dev[mBulletStruct[i].dir][1]) * SMALL_BOX_SIZE;
				mBombS[i].counter = 0;
				ClearWallOrStone(i, bombx, bomby );
				return true;
			}
		}
	}
		break;
	default:
		break;
	}
	return false;
}

// 子弹击中障碍物爆炸调用该函数, 击中边界不可调用, 下标会越界[52][52]
void PlayerBase::ClearWallOrStone(int bulletid, int bulletx, int bullety)
{
	int boxi = bullety / SMALL_BOX_SIZE;
	int boxj = bulletx / SMALL_BOX_SIZE;
	int tempx, tempy;
	switch (mBulletStruct[bulletid].dir)
	{
	case DIR_LEFT:
	case DIR_RIGHT:
	{
		// 相邻的四个 4*4 格子, 顺序不能变, 后面用到下标判断
		int temp[4][2] = { { -2, 0 },{ -1, 0 },{ 0, 0 },{ 1, 0 } };
		for (int i = 0; i < 4; i++)
		{
			tempx = boxi + temp[i][0];
			tempy = boxj + temp[i][1];
			if (bms->box_4[tempx][tempy] == _WALL || mPlayerTankLevel == 3 && bms->box_4[tempx][tempy] == _STONE)
				bms->box_4[tempx][tempy] = _CLEAR;

			// 转到 tempx,tempy所在的 8*8 格子索引
			int n = tempx / 2;
			int m = tempy / 2;

			// 检测 8*8 格子内的4个 4*4 的小格子是否全部被清除,
			bool isClear = true;	
			for (int a = 2 * n; a < 2 * n + 2; a++)
			{
				for (int b = 2 * m; b < 2 * m + 2; b++)
				{
					if (bms->box_4[a][b] != _CLEAR)
						isClear = false;
				}
			}
			if (isClear)
			{
				bms->box_8[n][m] = _EMPTY;
			}
		}
	}
	break;

	case DIR_UP:
	case DIR_DOWN:
	{
		// 相邻的四个 4*4 格子, 顺序不能变, 后面用到下标判断
		int temp[4][2] = { {0, -2}, {0, -1}, {0, 0}, {0, 1} };
		for (int i = 0; i < 4; i++)
		{
			tempx = boxi + temp[i][0];
			tempy = boxj + temp[i][1];
			if (bms->box_4[tempx][tempy] == _WALL || mPlayerTankLevel == 3 && bms->box_4[tempx][tempy] == _STONE)
				bms->box_4[tempx][tempy] = _CLEAR;

			// 转到 tempx,tempy所在的 8*8 格子索引
			int n = tempx / 2;
			int m = tempy / 2;

			// 检测 8*8 格子内的4个 4*4 的小格子是否全部被清除,
			bool isClear = true;
			for (int a = 2 * n; a < 2 * n + 2; a++)
			{
				for (int b = 2 * m; b < 2 * m + 2; b++)
				{
					if (bms->box_4[a][b] != _CLEAR)
						isClear = false;
				}
			}
			if (isClear)
			{
				bms->box_8[n][m] = _EMPTY;
			}
		}
	}
	break;

	default:
		break;
	}
}

// 参数是 16 * 16 中心点像素坐标, 与坦克中心坐标相同
void PlayerBase::SignBox_8(int x, int y, int val)
{
	// 右坦克中心索引转到左上角那个的 格子索引
	int iy = y / BOX_SIZE - 1;
	int jx = x / BOX_SIZE - 1;
	for (int i = iy; i < iy + 2; i++)
	{
		for (int j = jx; j < jx + 2; j++)
		{
			bms->box_8[i][j] = val;
		}
	}
}

