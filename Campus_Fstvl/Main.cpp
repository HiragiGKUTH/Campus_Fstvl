//////////////////////////////////////////////
//		For Campus Festival Game
//		Programmed by Hiragi and Sekiya
//      Created at 5/8
//		
//		Use OpenSiv3D Version 0.2.5
//		CC License BY SA NC
//////////////////////////////////////////////

#include <Siv3D.hpp> // OpenSiv3D v0.2.5
#include <HamFramework.hpp>
#include <memory>
#include "Define.h"


struct SharedData {
	int state;
	bool flag;
};

using MyGame = SceneManager<String, SharedData>;

class Title : public MyGame::Scene {
private:
	String _titleText = U"Prototype!!";
	Array<String> _menuText = {
		U"Game Start",
		U"Credit",
		U"Exit",
	};
	Array<Rect> _menuBoxes;
	Vec2 _basePos = Vec2(720, 380);
	Font _menuFont;
	Font _titleFont;
	Stopwatch _stopwatch;


public:
	Title(const InitData& init) : IScene(init) {
		_stopwatch.start();
		_menuFont = Font(48);
		_titleFont = Font(96);
		_menuBoxes.resize(_menuText.size());
		for (auto i : step(_menuBoxes.size())) {
				//箱の位置を指定
			_menuBoxes[i].set(_basePos.x + i * 30, _basePos.y + i * 100, _menuFont(_menuText[i]).region().w, _menuFont(_menuText[i]).region().h);
				//左右を少し拡張、上下もわずかに拡張
			_menuBoxes[i] = _menuBoxes[i].stretched(16,0);
		}
	}

	void update() override {
		for (auto i : step(_menuBoxes.size())) {
			const Quad quad = _menuBoxes[i].shearedX(4.0);
			
			if (quad.mouseOver()) {
				_menuBoxes[i] = _menuBoxes[i].stretched(4, 0);
				//_menuBoxes[i].set(_basePos.x + i * 30-30, _basePos.y + i * 100, _menuFont(_menuText[i]).region().w+60, _menuFont(_menuText[i]).region().h);
			}
			else {
				//_menuBoxes[i].set(_basePos.x + i * 30, _basePos.y + i * 100, _menuFont(_menuText[i]).region().w, _menuFont(_menuText[i]).region().h);
				//_menuBoxes[i] = _menuBoxes[i].stretched(16, 0);
			}
			if (i == 2) {

			}

			if (quad.leftClicked()) {
				switch (i) {
				case 0:
					changeScene(U"Game");
					break;
				case 1:
					changeScene(U"Credit");
					break;
				case 2:
					System::Exit();
					break;
				}
			}
		}
	}

	void draw() const override {

		_titleFont(_titleText).drawAt(Window::Center()-Point(200,200));

		for (auto i : step(_menuBoxes.size())) {
				//平行四辺形にして描画
			_menuBoxes[i].shearedX(4.0).draw();
			_menuFont(_menuText[i]).drawAt(_menuBoxes[i].center(),Palette::Black);
		}
	}
};

class Game : public MyGame::Scene {
private:
	String _text = U"Game Scene!";
	Font _font;
	//Font _debugFont;

	//Bullet class
	class Bullet {
	private:
		Font _debugFont;
		Vec2 _pos;
		Vec2 _v;
		Vec2 _target;
		double _ang;
		double _targetAng;
		int _targetCnt;
		double _speed;
		Circle _circle;
		bool _isDeleting = false;
		bool _isTracking = false;
		bool _isTracked = false;

	public:
		Bullet() {}
		void init(Vec2 pos,double speed,double angle,double r) {
			_targetCnt = 0;
			_pos = pos;
			_ang = angle;
			_speed = speed;
			_v = Vec2(Math::Cos(angle)*speed, Math::Sin(angle)*speed);
			_circle = Circle(_pos, r);
		}
		void update() {
			//追跡処理
			if (this->_isTracking && !this->_isTracked) {
				_targetCnt++;
				double _differAng;

				_targetAng = ((Math::HalfPi*5.0) - (Math::Atan2(_target.x - _pos.x, _target.y - _pos.y)));
				/*
				if (_targetAng < _ang) {
					_ang -= 0.1;
				}
				else {
					_ang += 0.1;
				}
				*/
				_v = Vec2(Math::Cos(_targetAng), Math::Sin(_targetAng))*_speed;
				if (_target.distanceFrom(_pos) < this->_speed*2){
					this->_isTracking = false;
					this->_isTracked = true;
				}
			}
		
				//移動量を反映
			_pos = _pos.moveBy(_v);
			_circle.setPos(_pos);
			
				//画面外で削除
			if (_pos.x < 0 || _pos.y < 0 || Window::Width() < _pos.x || Window::Height() < _pos.y) {
				_isDeleting = true;
			}
		}

		void draw() {
			//_debugFont(U"angle:", _targetAng).draw(_pos, Palette::White);
			_circle.draw(Palette::Wheat);
		}

		Circle getBullet() {
			return _circle;
		}

		void setTarget(Vec2 target) {
			_target = target;
			_isTracking = true;
		}

		void erase() { delete this; }
		bool isErasable() { return _isDeleting; }


	};

	//Player class
	class Player {
	private:
		Vec2 _pos;
		Vec2 _v;
		Circle _circle;
		Array<Bullet*> bullets;
		int _bulletcnt;
		Font _debugFont;
		bool bulletTracking = false;

		
		int _hp;
		const double speed = 10.0;
	public:
		Player() { _debugFont = Font(18); }

		void init(Vec2 pos) {
			_hp = 0;
			_v = Vec2::Zero();
			_pos = pos;
			_circle = Circle(_pos, 24);
			_bulletcnt = 0;
		}

		void update() {
			_bulletcnt = 0;
			_v = Vec2::Zero();

			//const auto joyR = JoyCon(Gamepad(0));
			const auto joyL = JoyCon(Gamepad(1));


			if(auto d = joyL.povD8())
				_v = Vec2(Math::Cos(*d*Radians(45)),Math::Sin(*d*Radians(45)))*speed;

			if (KeyRight.pressed())
				_v = Vec2::Right()*speed;
			if (KeyLeft.pressed())
				_v = Vec2::Left()*speed;
			if (KeyUp.pressed())
				_v = Vec2::Up()*speed;
			if (KeyDown.pressed())
				_v = Vec2::Down()*speed;



			if (joyL.buttonStick.down() || KeyX.down())
				bulletTracking = bulletTracking ? false : true;

			if (bulletTracking) {
				for (const auto& bullet : bullets) {
					bullet->setTarget(Vec2(1280/2, 720/2));
				}
			}

				//Enter Bullets
			if (joyL.buttonLR.pressed() || KeyZ.pressed() && System::FrameCount()%10 == 0) {
				for (auto i : step(3)) {
					Bullet *tmp = new Bullet();
					tmp->init(_pos, 16.0, (Math::Pi * 6 / 4)+(i-1)*0.1, 8);
					bullets.push_back(tmp);
				}
			}


			for (auto itr = bullets.begin(); itr != bullets.end();) {
				if ((*itr)->isErasable()) {
					(*itr)->erase();
					itr = bullets.erase(itr);
				} else {
					_bulletcnt++;
					(*itr)->update();
					itr++;
				}
			}

			_pos = _pos.moveBy(_v);
			_circle.setPos(_pos);
		}

		void draw() const {
			for (const auto& bullet : bullets) {
				bullet->draw();
			}
			_circle.draw();
			_debugFont(_bulletcnt, U"Bullets").draw(0, 0,Palette::White);
		}

		void hit() { _hp -= 5; }
		Array<Bullet*> getBullets() { return bullets; }
		
		
	};

	//Invader class
	class Invader {
	private:
		Vec2 _pos;
		Vec2 _v;
		Circle _circle;
		ColorF _inner;
		ColorF _outer;
		Rect _hpBar;
		Array<Bullet*> bullets;
		Bullet *tmp;
		Font _debugFont;
		int _hp;
		int _hpMax;
		int _bulletcnt;
		int _cnt;
		int _shotPattern;
		int _movePattern;


		const double speed = 10.0;
	public:
		Invader() { _debugFont = Font(18); }

		ColorF GetBarColor(double rate) const
		{
			if (rate > 0.3)
			{
				return HSV(120, 1.0, 0.9);
			}
			else if (rate > 0.1)
			{
				return HSV(40, 1.0, 0.9);
			}
			else
			{
				return HSV(0, 1.0, 0.9);
			}
		}

		void init(Vec2 pos,int shotPattern,int movePattern) {
			_cnt = 0;
			_v = Vec2::Zero();
			_pos = pos;
			_circle = Circle(_pos, 18);
			_bulletcnt = 0;
			_shotPattern = shotPattern;
			_movePattern = movePattern;

			_hpBar = Rect(_pos.x, _pos.y, 64, 16);

			switch (shotPattern) {
				case 0:
					_outer = Palette::Red;
					break;
				case 1:
					_outer = Palette::Aqua;
					break;
				case 2:
					_outer = Palette::Brown;
					break;
				case 3:
					_outer = Palette::Yellow;
					break;
			}

			switch (movePattern) {
				case 0:
					_inner = Palette::Hotpink;
					_hp = 200;
					break;
				case 1:
					_inner = Palette::Green;
					_hp = 150;
					break;
				case 2:
					_inner = Palette::White;
					_hp = 100;
					break;
				case 3:
					_inner = Palette::Coral;
					_hp = 400;
					break;
			}
			_hpMax = _hp;
		}

		void update() {
			_cnt++;
			//Tick Movements
			switch (_movePattern) {
				case 0: {
					_v = Vec2(0, 1);
					
					break;
				}

				case 1: {
					_v = Vec2(3, 1);
					if (_pos.x > Window::Width())
						_pos.x = 0;
					break;
				}

				case 2: {
					_v = Vec2(-3, 1);
					if (_pos.x < 0)
						_pos.x = Window::Width();
					break;
				}

				case 3: {
					_v = Vec2(0, 0.5);
					if (_pos.x > Window::Width())
						_pos.x = 0;
					break;
				}
			}

			//Shot Movements
			switch (_shotPattern) {
				case 0: {
					if (_cnt % 60 == 0) {
						for (auto i : step(36)) {
							tmp = new Bullet();
							tmp->init(_pos, 10.0, Radians(10 * i), 8);
							bullets.push_back(tmp);
						}
					}
					break;
				}

				case 1: {
					if (_cnt % 30 == 0) {
						for (auto i : step(3)) {
							double target;
							tmp = new Bullet();
							tmp->init(_pos, 10.0, Radians((10 * i)-10+90), 8);
							bullets.push_back(tmp);
						}
					}
					break;
				}

				case 2: {
					if (_cnt % 2 == 0) {
						double target;
						tmp = new Bullet();
						tmp->init(_pos, 10.0, Radians((4 * (_cnt%5)) - 6 + 90), 8);
						bullets.push_back(tmp);
						
					}
					break;
				}

				case 3: {
					if (_cnt % 30 == 0) {
						for (auto i : step(3)) {
							double target;
							tmp = new Bullet();
							tmp->init(_pos, 10.0, Radians((10 * i) - 10 + 90), 8);
							bullets.push_back(tmp);
						}
					}
					break;
				}
			}
			
			//shot process
			for (auto itr = bullets.begin(); itr != bullets.end();) {
				if ((*itr)->isErasable()) {
					(*itr)->erase();
					itr = bullets.erase(itr);
				}
				else {
					_bulletcnt++;
					(*itr)->update();
					itr++;
				}
			}

			//update position
			_pos = _pos.moveBy(_v);
			_circle.setPos(_pos);
			//Hp Bar
			_hpBar = Rect(_pos.x+20, _pos.y,(_hpMax - (_hpMax-_hp))/3, 16);
			_debugFont(U"Hp:", _hp).draw(_pos + Vec2(30, 0));
		}

		void draw() const {
			_circle.draw(_inner);
			_circle.drawFrame(2.0,_outer);
			for (const auto& bullet : bullets) {
				bullet->draw();
			}
			_hpBar.draw(GetBarColor((double)_hp / _hpMax));

		}

		void hit(int damage) { _hp -= damage; }

		Circle getPos() { return _circle; }
		Array<Bullet*> getBullets(){ return bullets; }
	};
	//Invader commander class
	class InvaderCommander {
	private:
		Array<Invader*> _invaders;
		Vec2 _pos;
		Vec2 _v;
		Circle _portal;
		Circle _portalCore;
		Font _font;
		double _speed;
		int _cooltime;
		int _coolcnt;
		int _max;
		int _invcnt;
		int _shotPattern;
		int _movePattern;

	public:
		InvaderCommander() {}
		void init(Vec2 pos,double r,int max) {
			_portal = Circle(pos, r);
			_portalCore = Circle(pos, r / 8.0);
			_pos = pos;
			_v = Vec2::Zero();
			_cooltime = 60;
			_coolcnt = 0;
			_max = max+9000;
			_invcnt = 0;
			_speed = 8;
			_font = Font(24);
			_shotPattern = 0;
			_movePattern = 0;
		}

		void update() {
			const auto joyR = JoyCon(Gamepad(0));

			//:::::::::::::::::::::: Movements ::::::::::::::::::::::::::::
			_v = Vec2::Zero();
			if (auto d = joyR.povD8())
				_v = -Vec2(Math::Cos(*d*Radians(45)), Math::Sin(*d*Radians(45)))*_speed;

			if (KeyD.pressed())
				_v = Vec2::Right()*_speed;
			if (KeyA.pressed())
				_v = Vec2::Left()*_speed;
	
			if (joyR.button0.down() || Key1.pressed())
				_shotPattern += _shotPattern < 3 ? 1 : 0;

			if (joyR.button3.down() || Key2.pressed())
				_shotPattern -= _shotPattern > 0 ? 1 : 0;

			if (joyR.button1.down() || Key3.pressed())
				_movePattern += _movePattern < 3 ? 1 : 0;

			if (joyR.button2.down() || Key4.pressed())
				_movePattern -= _movePattern > 0 ? 1 : 0;


			_v.y = 0;
			_pos = _pos.moveBy(_v);
			_portal.setPos(_pos);
			_portalCore.setPos(_pos);


			//::::::::::::::::::::: Invader Register ::::::::::::::::::::::
			_coolcnt+=1;

			if (joyR.buttonLR.pressed() || Key0.pressed() && _coolcnt > _cooltime && _invcnt <= _max) {
				_coolcnt = 0;
				_invcnt++;

				Invader *tmp = new Invader();
				tmp->init(_pos, _shotPattern, _movePattern);
				_invaders.push_back(tmp);
			}


			//:::::::::::::::::::: Update Invaders ::::::::::::::::::::::::
			for (auto invader : _invaders) {
				invader->update();
			}
		}

		void draw() const {
			//:::::::::::::::::::: Draw Invaders ::::::::::::::::::::::
			for (auto invader : _invaders) {
				invader->draw();
			}

			_portal.drawFrame(2.0, Palette::Aqua);
			_portalCore.draw(Palette::Red);
			_font(U"Move:", _movePattern).drawAt(Vec2(_pos.x + 90, _pos.y - 20),Palette::White);
			_font(U"Shot:", _shotPattern).drawAt(Vec2(_pos.x + 90, _pos.y + 20), Palette::White);
		}

		Array<Invader*> getInvaders() { return _invaders; }
	};
	Player _player;
	InvaderCommander _invadercommander;

public:
	Game(const InitData& init) : IScene(init) {
		_player.init(Vec2(Window::Width()/2,600));
		_invadercommander.init(Vec2(Window::Width() / 2, 40), 32, 8);
		_font = Font(50);
	//	_debugFont = Font(18);
	}

	void update() override {
		_player.update();
		_invadercommander.update();

		for (auto inv : _invadercommander.getInvaders()) {
			for (auto bul : _player.getBullets()) {
				if(bul->getBullet().intersects(inv->getPos())) {
					inv->hit(5);
				}
			}
		}

		if (MouseR.pressed()) {
			changeScene(U"Title");
		}
	}

	void draw() const override {
		_player.draw();
		_invadercommander.draw();
	}
};

class Credit : public MyGame::Scene {
private:
	String _text = U"Credit Scene!";
	Font _font;
public:
	Credit(const InitData& init) : IScene(init) {
		_font = Font(50);
	}

	void update() override {
		if (MouseL.pressed()) {
			changeScene(U"Title");
		}
	}

	void draw() const override {
		_font(_text).draw(0, 0, Palette::Aqua);
	}
};

void Main()
{
		//System Initialize
	Window::Resize(WIDTH, HEIGHT);
	Graphics::SetTargetFrameRateHz(REFRESHRATE);

	MyGame manager;
	manager.add<Title>(U"Title");
	manager.add<Game>(U"Game");
	manager.add<Credit>(U"Credit");
	while (System::Update())
	{
		manager.update();
	}
}
