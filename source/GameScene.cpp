#include "GameScene.h"

using namespace SFAS2014;

GameScene::GameScene(int noOfPlayers, float xGraphicsScale, float yGraphicsScale, SettingsMenu* settingsMenu, const float starSize)
	:MasterScene(xGraphicsScale, yGraphicsScale, settingsMenu), m_Time((float) keTimeLimit), kGoldProb(0.3), kSilverProb(0.2),
	m_GameState(keGamePlaying), m_NoOfPlayers(noOfPlayers), kStarSize(starSize)
{
	Create();
}

void GameScene::Create()
{
	//Set up vectors so that they represent the correct number of players
	m_ScoreLabel.reserve(m_NoOfPlayers);
	m_TimeLabel.reserve(m_NoOfPlayers);
	m_DoublePoints.reserve(m_NoOfPlayers);
	m_TriplePoints.reserve(m_NoOfPlayers);
	m_Delayed.reserve(m_NoOfPlayers);
	m_Grid.reserve(m_NoOfPlayers);
	m_FirstSelectedItem.reserve(m_NoOfPlayers);
	m_SecondSelectedItem.reserve(m_NoOfPlayers);
	m_CharactersToRemove.reserve(m_NoOfPlayers);

	m_NoOfMatchedPairs.reserve(m_NoOfPlayers);
	m_DoublePointsTimer.reserve(m_NoOfPlayers);
	m_TriplePointsTimer.reserve(m_NoOfPlayers);
}

void GameScene::Update(float deltaTime, float alphaMul)
{
	//if not current scene, don't bother updating.
	if(!(m_Manager->GetCurrent() == this))
	{
		return;
	}
	Scene::Update(deltaTime, alphaMul);

	if( m_IsInputActive && !g_pInput->m_Touched && g_pInput->m_PrevTouched)
	{
		SettingsMenuHitTest();
	}

	//call the right method for the correct game state
	switch(m_GameState)
	{
	case keGameOver:
		m_GameState = keGamePlaying;
		break;
	case keGamePlaying:
		for(int i = 0; i < m_NoOfPlayers; i++)
		{
			if(m_Delayed[i])
			{
				DelayGameForNonmatch(deltaTime, i);
			}
		}
		CheckForAnyMatches();
		UpdateTime(deltaTime);
		UpdateLabels();
		break;
	}
	
	//If the user has clicked elsewhere just swallow the touch
	if( m_IsInputActive && !g_pInput->m_Touched && g_pInput->m_PrevTouched)
	{
		g_pInput->Reset();
	}
}

void GameScene::CheckForAnyMatches()
{
	if( m_IsInputActive && !g_pInput->m_Touched && g_pInput->m_PrevTouched)
	{
		g_pInput->Reset();
		for(int i = 0; i < m_NoOfPlayers; i++)
		{
			CheckGridForMatch(i);
		}
		
	}
}

void GameScene::CheckGridForMatch(int player)
{
	if(!m_Delayed[player])
	{
		for(int gridIndex = 0; gridIndex < keGridWidth*keGridHeight; gridIndex++ )
	{
		if(StarHasBeenTouched(m_Grid[player][gridIndex]))
		{
			//if the star has been touched then show the character underneath. If the player has already selected a star
			//then see if we have a match or not. Else set m_FirstSelectedItem to the touched star
			ShowCharacter(m_Grid[player][gridIndex]);
			if(m_FirstSelectedItem[player])
			{
				m_SecondSelectedItem[player] = m_Grid[player][gridIndex];
				//If the character indexes match up, then we have a normal, silver or gold match
				if(m_FirstSelectedItem[player]->GetCharacterIndex() ==  m_Grid[player][gridIndex]->GetCharacterIndex())
				{
					ProcessMatch(player);
				}
				//If the character indexes don't match, but the player has selected two characters which are both gold or both silver, then we have an odd match,
				//which we need to handle a little differently.
				else if((m_FirstSelectedItem[player]->IsGold() && m_SecondSelectedItem[player]->IsGold()) || (m_FirstSelectedItem[player]->IsSilver() && m_SecondSelectedItem[player]->IsSilver()))
				{
					ProcessOddPowerupMatch(player);
				}
				//Else we have no match
				else
				{
					ProcessIncorrectMatch(player);
				}
			}
			else
			{
				m_FirstSelectedItem[player] = m_Grid[player][gridIndex];
			}	
			break;
		}
	}
	}
	
}

void GameScene::ProcessMatch(int player)
{
	//Both characters have to both be gold, both be silver, or both be the same normal character
	if((m_FirstSelectedItem[player]->IsGold() == m_SecondSelectedItem[player]->IsGold())  && (m_FirstSelectedItem[player]->IsSilver() == m_SecondSelectedItem[player]->IsSilver()))
	{
		if(m_FirstSelectedItem[player]->IsGold())
		{
			ProcessGoldMatch(player);
		}
		else if(m_FirstSelectedItem[player]->IsSilver())
		{
			ProcessSilverMatch(player);
		}
		else
		{
			ProcessNormalMatch(player);
		}
					
		RemoveCharactersAfterDelay(player);
	}
}

void GameScene::IncrementScore(int amount, int player)
{
	((GameSceneManager*) m_Manager)->IncrementScore(amount, player);
}

GameScene::~GameScene()
{
	for(int i = 0; i < m_NoOfPlayers; i++)
	{
		delete m_DoublePointsTimer[i];
		delete m_TriplePointsTimer[i];
	}

	for(int i = 0; i < keGridHeight*keGridWidth; i++)
	{
		for(int j = 0; j < m_NoOfPlayers; j++)
		{
			if(m_Grid[j][i] != 0)
			{
				delete m_Grid[j][i];
			}
		}
	}
}

void GameScene::ProcessSilverMatch(int player)
{
	g_pAudio->PlaySound(g_pResources->GetSilverPickupSoundFilename());
	
	if(m_DoublePointsTimer[player] != NULL)
	{
		m_Timers.Cancel(m_DoublePointsTimer[player]);
		m_DoublePointsTimer[player] = NULL;
		
	}
	if(m_TriplePoints[player])
	{
		IncrementScore(60, player);
	}
	else if(m_DoublePoints[player])
	{
		IncrementScore(40, player);
	}
	else
	{
		IncrementScore(20, player);
	}
	m_DoublePoints[player] = true;
	if(0 == player)
	{
		m_DoublePointsTimer[player] = new Timer(10.0f, 1, &GameScene::reset_player_1_double_points, (void*)this);
		m_Timers.Add(m_DoublePointsTimer[player]);
	}
	else
	{
		m_DoublePointsTimer[player] = new Timer(10.0f, 1, &GameScene::reset_player_2_double_points, (void*)this);
		m_Timers.Add(m_DoublePointsTimer[player]);
	}
	
}

void GameScene::ProcessGoldMatch(int player)
{
	g_pAudio->PlaySound(g_pResources->GetGoldPickupSoundFilename());
	m_TriplePoints[player] = true;
	if(m_TriplePointsTimer[player] != NULL)
	{
		m_Timers.Cancel(m_TriplePointsTimer[player]);
		m_TriplePointsTimer[player] = NULL;
		IncrementScore(150, player);
	}
	if(m_DoublePoints[player])
	{
		IncrementScore(100, player);
	}
	else
	{
		IncrementScore(50, player);
	}

	if(0 == player)
	{
		m_TriplePointsTimer[player] = new Timer(10.0f, 1, &GameScene::reset_player_1_triple_points, (void*)this);
		m_Timers.Add(m_TriplePointsTimer[player]);
	}
	else
	{
		m_TriplePointsTimer[player] = new Timer(10.0f, 1, &GameScene::reset_player_2_triple_points, (void*)this);
		m_Timers.Add(m_TriplePointsTimer[player]);
	}
}

void GameScene::DelayGameForNonmatch(float deltaTime, int player)
{
	if(m_DelayTime[player] <= 0)
	{
		if(m_FirstSelectedItem[player]->IsGold() || m_FirstSelectedItem[player]->IsSilver())
		{
			RemovePairsPowerUp(m_FirstSelectedItem[player], player);
			
		}
		else if(m_SecondSelectedItem[player]->IsGold() || m_SecondSelectedItem[player]->IsSilver())
		{
			RemovePairsPowerUp(m_SecondSelectedItem[player], player);
		}

		HideCharacter(m_FirstSelectedItem[player]);
		m_FirstSelectedItem[player] = NULL;

		HideCharacter(m_SecondSelectedItem[player]);
		m_SecondSelectedItem[player] = NULL;

		m_DelayTime[player] = 0;

		m_Delayed[player] = false;
	}
	else
	{
		m_DelayTime[player] -= deltaTime;	
	}
}

void GameScene::RemoveMatchedCharacterPairFromList(int player)
{
	GridItem * char1;
	GridItem * char2;

	char1 = m_CharactersToRemove[player].at(0);
	char2 = m_CharactersToRemove[player].at(1);
	m_CharactersToRemove[player].erase(m_CharactersToRemove[player].begin());
	m_CharactersToRemove[player].erase(m_CharactersToRemove[player].begin());

	RemoveChild(char1->GetCharacterSprite());
	RemoveChild(char2->GetCharacterSprite());
	
}

void GameScene::reset_player_1_double_points(Timer* timer, void* userData)
{
	GameScene* self = (GameScene*) userData;
	self->m_DoublePoints[0] = false;
}

void GameScene::reset_player_2_double_points(Timer* timer, void* userData){
	GameScene* self = (GameScene*) userData;
	self->m_DoublePoints[1] = false;
}

void GameScene::reset_player_1_triple_points(Timer* timer, void* userData)
{
	GameScene* self = (GameScene*) userData;
	self->m_TriplePoints[0] = false;
}

void GameScene::reset_player_2_triple_points(Timer* timer, void* userData){
	GameScene* self = (GameScene*) userData;
	self->m_TriplePoints[1] = false;
}

void GameScene::remove_player_1_matched_characters(Timer* timer, void* userData)
{
	GameScene * self = (GameScene*)userData;
	//Remove characters from scene and from list of characters to remove.
	self->RemoveMatchedCharacterPairFromList(0);

	self->m_NoOfMatchedPairs[0]++;

	//If the player has matched 12 pairs then reset the board.
	if(((keGridWidth*keGridHeight)/2) == self->m_NoOfMatchedPairs[0])
	{
		g_pAudio->PlaySound(g_pResources->GetBoardCompleteSoundFilename());
		self->m_NoOfMatchedPairs[0] = 0;
		self->ResetBoard(0);
	}
	
}

void GameScene::remove_player_2_matched_characters(Timer* timer, void* userData)
{
	GameScene * self = (GameScene*)userData;
	//Remove characters from scene and from list of characters to remove.
	self->RemoveMatchedCharacterPairFromList(1);

	self->m_NoOfMatchedPairs[1]++;

	//If the player has matched 12 pairs then reset the board.
	if(((keGridWidth*keGridHeight)/2) == self->m_NoOfMatchedPairs[1])
	{
		g_pAudio->PlaySound(g_pResources->GetBoardCompleteSoundFilename());
		self->m_NoOfMatchedPairs[1] = 0;
		self->ResetBoard(1);
	}
	
}

bool GameScene::StarHasBeenTouched(GridItem * gridItem)
{
	return (gridItem->GetStarSprite()->m_IsVisible && gridItem->GetStarSprite()->HitTest( g_pInput->m_X, g_pInput->m_Y )); 
}

void GameScene::ProcessNormalMatch(int player)
{
	g_pAudio->PlaySound(g_pResources->GetMatchSoundFilename());
	if(m_TriplePoints[player])
	{
		IncrementScore(30, player);
	}
	else if(m_DoublePoints[player])
	{
		IncrementScore(20, player);
	}
	else
	{
		IncrementScore(10, player);
	}
	
}

void GameScene::ProcessIncorrectMatch(int player)
{
	g_pAudio->PlaySound(g_pResources->GetNonmatchSoundFilename());
	m_DelayTime[player] = 0.5f;
	m_Delayed[player] = true;
}

void GameScene::ProcessOddPowerupMatch(int player)
{
	//This is for the case where the player has matched up two gold or silver characters, but these characters have underlying different character indexes,
	//meaning that when they are matched and removed they will leave odd corresponding pairs. The second left over character is changed to have the same
	//character index as the first left over character, so that there are no odd characters left on the board.
	CharacterBuilder charToMake(m_FirstSelectedItem[player]->GetCharacterIndex());
	charToMake.SetGold(m_FirstSelectedItem[player]->IsGold());
	charToMake.SetSilver(m_FirstSelectedItem[player]->IsSilver());
	FindOtherHalfOfPair(m_FirstSelectedItem[player], player)->RemovePowerup();
	FindOtherHalfOfPair(m_SecondSelectedItem[player], player)->SetCharacterImage(charToMake);

	//Then process match like normal
	ProcessMatch(player);
}

GridItem * GameScene::FindOtherHalfOfPair(GridItem* gridItem, int player)
{
	for(int i = 0; i < keGridHeight*keGridWidth; i++)
	{
		if(gridItem->GetCharacterIndex() == m_Grid[player][i]->GetCharacterIndex() && m_Grid[player][i] != gridItem)
		{
			return m_Grid[player][i];
		}
	}
	return NULL;
}

void GameScene::InitBoard()
{
	for(int i = 0; i < m_NoOfPlayers; i++)
	{
		std::vector<CharacterBuilder> characterTypes;
		SetupCharactersArray(characterTypes);
		AddGridToScene(characterTypes, i);
	}
	std::vector<CharacterBuilder> player1CharacterTypes;
	SetupCharactersArray(player1CharacterTypes);	
	AddGridToScene(player1CharacterTypes, 0);

	std::vector<CharacterBuilder> player2CharacterTypes;
	SetupCharactersArray(player2CharacterTypes);	
	AddGridToScene(player2CharacterTypes, 1);
}

void GameScene::ResetBoard(int player)
{
	std::vector<CharacterBuilder> characterTypes;
	SetupCharactersArray(characterTypes);	
	ResetGrid(characterTypes, player);
}

void GameScene::ResetGrid(std::vector<CharacterBuilder> characterTypes, int player)
{
	for(int i = 0; i < keGridHeight*keGridWidth; i++)
	{
		int characterTypeIndex = rand() % characterTypes.size();
		
		CharacterBuilder charToMake = characterTypes.at(characterTypeIndex);
		m_Grid[player][i]->GetStarSprite()->m_IsVisible = true;
		m_Grid[player][i]->SetCharacterImage(charToMake);

		float characterScale = kStarSize / (m_Grid[player][i]->GetCharacterSprite()->m_H);
		m_Grid[player][i]->GetCharacterSprite()->m_ScaleX = characterScale*m_XGraphicsScale;
		m_Grid[player][i]->GetCharacterSprite()->m_ScaleY = characterScale*m_YGraphicsScale;

		AddChild(m_Grid[player][i]->GetCharacterSprite());

		characterTypes.erase(characterTypes.begin() + characterTypeIndex);
	}
}

void GameScene::PauseGame()
{
	m_GameState = keGamePause;
	for(int i = 0; i < m_NoOfPlayers; i++)
	{
		if(m_DoublePointsTimer[i])
		{
			m_DoublePointsTimer[i]->Pause();
		}
		if(m_TriplePointsTimer[i])
		{
			m_DoublePointsTimer[i]->Pause();
		}
	}
}

void GameScene::ResumeGame()
{
	m_GameState = keGamePlaying;
	for(int i = 0; i < m_NoOfPlayers; i++)
	{
		if(m_DoublePointsTimer[i])
		{
			m_DoublePointsTimer[i]->Resume();
		}
		if(m_TriplePointsTimer[i])
		{
			m_DoublePointsTimer[i]->Resume();
		}
	}
}

void GameScene::RemovePairsPowerUp(GridItem * selected, int player)
{
	selected->RemovePowerup();
	int charIndex = selected->GetCharacterIndex();

	FindOtherHalfOfPair(selected, player)->RemovePowerup();
	//Find the other half of the powerup pair and remove it.
	
}

bool GameScene::AMinuteHasGoneBy(float deltaTime)
{
	return ((0 == ((int)m_Time % 60)) && ((int)m_Time > (int)(m_Time - deltaTime)));
}

bool GameScene::InTheFinal10Seconds(float deltaTime)
{
	return ((m_Time <= 11) && ((int)m_Time > (int)(m_Time - deltaTime)));
}

void GameScene::UpdateTime(float deltaTime)
{
	// If a minute has gone by, or we're in the final 10 seconds of the game, then beep to alert the user.
	if((m_Time < keTimeLimit) && 
		AMinuteHasGoneBy(deltaTime) || 
		InTheFinal10Seconds(deltaTime))
	{
		g_pAudio->PlaySound(g_pResources->GetTimeSoundFilename());
	}
	
	//If there is no time left, then clean up a few variables and change to the results scene
	if( ( m_Time) <= 0 )
	{
		ExitScene();
	}

	//Update the timer
	m_Time -= deltaTime;
}

void GameScene::UpdateLabels()
{
	int minutes, seconds;
	minutes = (int)( m_Time / 60 );
	seconds = (int)( m_Time - ( minutes * 60.0f ) );
	char timeBuffer[256];
	sprintf(timeBuffer, "%.2d:%.2d", minutes, seconds );

	for(int i = 0; i < m_NoOfPlayers; i++)
	{
		char scoreBuffer[5];
		sprintf(scoreBuffer, "%.4d", ((GameSceneManager*) m_Manager)->GetScore(i) );
		m_ScoreLabel[i]->SetText(scoreBuffer);
		m_TimeLabel[i]->SetText(timeBuffer);
	}
}

void GameScene::SetupCharactersArray(std::vector<CharacterBuilder> &characterTypes)
{
	//Fill vector with exactly 2 of each character type
	int numberOfPairs = (keGridHeight*keGridWidth)/2;
	for(int i = 0; i < numberOfPairs; i++)
	{
		CharacterBuilder charToMake(i);
		float randNum = ((float) rand() / RAND_MAX );
		if(randNum <= kGoldProb)
		{
			charToMake.SetGold(true);
		}
		else if(randNum <= kSilverProb)
		{
			charToMake.SetSilver(true);
		}
		characterTypes.push_back(charToMake);
		characterTypes.push_back(charToMake);
	}
}

void GameScene::RemoveCharactersAfterDelay(int player)
{
	m_CharactersToRemove[player].push_back(m_FirstSelectedItem[player]);
	m_CharactersToRemove[player].push_back(m_SecondSelectedItem[player]);
	if(0 ==player)
	{
		m_Timers.Add(new Timer(0.5f, 1, &GameScene::remove_player_1_matched_characters, (void*)this));
	}
	else
	{
		m_Timers.Add(new Timer(0.5f, 1, &GameScene::remove_player_2_matched_characters, (void*)this));
	}
	m_FirstSelectedItem[player] = NULL;
	m_SecondSelectedItem[player] = NULL;
	
}

void GameScene::HideCharacter(GridItem * gridItem)
{
	gridItem->GetCharacterSprite()->m_IsVisible = false;
	gridItem->GetStarSprite()->m_IsVisible = true;
}

void GameScene::ShowCharacter(GridItem * gridItem)
{
	gridItem->GetStarSprite()->m_IsVisible = false;
	gridItem->GetCharacterSprite()->m_IsVisible = true;
}

void GameScene::Init()
{
	MasterScene::Init();

	//Initialise labels for score and time
	InitLabels();

	//Initialise the game board
	InitBoard();
}

void GameScene::Reset()
{
	MasterScene::Reset();
	if(keGameOver == m_GameState)
	{
		for(int i = 0; i < m_NoOfPlayers; i++)
		{
			ResetBoard(i);
		}
	}
	
	m_Time = (float)keTimeLimit;
	for(int i = 0; i < m_NoOfPlayers; i++)
	{
		((GameSceneManager*) m_Manager)->SetScore(0, i);
	}
	UpdateLabels();
}