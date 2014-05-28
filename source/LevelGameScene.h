#pragma once
/*
 * (C) 2014 Search for a Star
 * 
 */

#include "GameScene.h"
#include "ResultsScene.h"

namespace SFAS2014
{
	
class GridItem;

/**
 * @class LevelGameScene
 *
 * @brief Displays and updates the game
 *
 */
class LevelGameScene : public GameScene
{

public:
	LevelGameScene(float xGraphicsScale, float yGraphicsScale, SettingsMenu * settingMenu);

private: 
	
	//Labels
	CLabel * m_UpdateToScoreLabel;
	CLabel * m_UpdateToTimeLabel;

	//Dialog boxes
	CSprite * m_GameStartDialog;
	CSprite * m_GameOverDialog;
	CSprite * m_LevelWonDialog;
	CSprite * m_ExitGameDialog;

	//Init Helpers
	void InitLabels();
	void InitButtons();
	void InitUI();
	void InitDialogs();
	void InitLevels();
	void InitGameOverDialog();
	void InitGameStartDialog();
	void InitLevelWonDialog();
	void InitExitDialog();
	void InitBoard();

	//Powerup helpers
	void RemovePairsPowerUp(GridItem* selected);
	static void ResetDoublePoints(Timer* timer, void* userData);

	//Update scene helpers
	void ResetBoard(int player);
	void ResetGrid(std::vector<CharacterBuilder> characterTypes, int player);
	void SetupCharactersArray(std::vector<CharacterBuilder> &characterTypes);
	void AddGridToScene(std::vector<CharacterBuilder> &characterTypes, int player);
	static void remove_player_1_matched_characters(Timer* timer, void* userData);
	void CheckGridForMatch(int player);
	void RemoveCharactersAfterDelay(int player);

	//Endgame helpers
	void ExitScene();

	//consts for labels and stars
	static const float kTimeTextX;
	static const float kTimeTextY;

	static const float kScoreTextX;
	static const float kScoreTextY;

	static const float kLabelHeight;
	static const float kLabelWidth;

	static const float kStarXOffset;
	static const float kStarYOffset;
	static const float kStarSpacing;
	static const float kButtonSpacing;
	static const float kButtonStartingX;
	static const float kButtonStartingY;

	static const float kUpdateToScoreX;
	static const float kUpdateToScoreY;
	static const float kUpdateToScoreHeight;
	static const float kUpdateToScoreWidth;

	static const float kUpdateToTimeX;
	static const float kUpdateToTimeY;
	static const float kUpdateToTimeHeight;
	static const float kUpdateToTimeWidth;

	struct Level 
	{
		Level(int time, int matches, int pairs) {timeLimit = time; noOfMatchesToWin = matches; noOfPairsPerBoard = pairs; }; 
		int timeLimit; //seconds
		int noOfMatchesToWin;
		int noOfPairsPerBoard;
		static const int ratioLookUp[][2];
	};

	const int noOfLevels;
	std::vector<Level> m_Levels;
	int m_ActiveLevel;
};
}



