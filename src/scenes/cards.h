/*
	CardsScene
	mperron (2019)

	A place to play cards.
*/
class CardsScene : public Scene {
	vector<PlayingCard*> card_deck;

public:
	CardsScene(Scene::Controller *ctrl) : Scene(ctrl) {
		// FIXME debug - change this image to a playing card table.
		bg = textureFromBmp(rend, "pict0007.bmp");

		PlayingCard::Suit suits[4] = {
			PlayingCard::Suit::A,
			PlayingCard::Suit::B,
			PlayingCard::Suit::C,
			PlayingCard::Suit::D
		};
		for(int suit_i = 0; suit_i < 4; suit_i++){
			for(int i = 1; i <= 13; i++){
				PlayingCard *card = new PlayingCard(
					rend,
					20 + (i * 10) + (suit_i * 15),
					20 + (10 - (i % 3) * 3) + (suit_i * 20),
					suits[suit_i],
					i,
					true
				);

				card_deck.push_back(card);
				drawables.push_back(card);
				clickables.push_back(card);
			}
		}
	}

	~CardsScene(){
		for(PlayingCard* card : card_deck)
			delete card;
	}
};
