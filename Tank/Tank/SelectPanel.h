#pragma once

// 指示玩家选择
enum EnumSelectResult
{
	OnePlayer = 1,	// 1->1个玩家
	TwoPlayer,		// 2->2个玩家
	Custom,			// 3->自定义地图
	Error			// 4
};

class SelectPanel
{
public:
	SelectPanel(Graphics* grap, HDC des_hdc, HDC image_hdc);
	~SelectPanel();

	void Init();
	EnumSelectResult ShowSelectPanel();			// 显示玩家选择面板, 返回玩家选择0,1,2, -1出错
private:
	void ShowStage();

private:
	Graphics* mGraphics;
	HDC mDes_hdc, mImage_hdc;

	// 上升动画,选择单人\双人\自定义\游戏.
	Image* mSelect_player_image;
	int mSelect_player_image_y;		// 图片从底部上升控制,初始值=672, 窗口底部

	// 开始选玩家1，2，地图
	byte mCounter : 1;				// 用来切换 mSelectTankImage 下标
	Image* mSelectTankImage[2];		// 指示器坦克, 从玩家坦克里面获取图片
	POINT mSelectTankPoint[3];		// 三个选项坐标
	int mSelectIndex;				// 选择结果: 0,1,2

	// 选择玩家后显示关卡信息
	Image* mGrayBackgroundImage;	// 游戏灰色背景图
	Image* mCurrentStageImage;		// 选择完后显示当前关卡
	IMAGE mBlackNumberImage;		// 数字 1234567890 图片
};