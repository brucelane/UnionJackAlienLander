//  Created by Andrew Morton https://github.com/drewish/AlienLander

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Utilities.h"
#include <boost/format.hpp>
#include "Resources.h"
#include "SegmentDisplay.h"
#include "Ship.h"
// UserInterface
#include "CinderImGui.h"
// parameters
#include "ParameterBag.h"
// audio
#include "AudioWrapper.h"
// spout
#include "SpoutWrapper.h"
// Utils
#include "Batchass.h"
// Console
#include "AppConsole.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace Reymenta;

class UnionJackApp : public App {

public:

	void setup() override;
	void cleanup() override;
	void draw() override;
	void update() override;
	void mouseDown(MouseEvent event) override;
	void mouseMove(MouseEvent event) override;
	void touchesMoved(TouchEvent event) override;
	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;
	void buildMeshes();
	void resize();
	unsigned int mPoints = 50;
	unsigned int mLines = 50;
	bool mShowHud = true;
	bool mShowCompass = false;

	Ship mShip;
	vector<SegmentDisplay> mDisplays;

	gl::BatchRef    mLineBatch;
	gl::BatchRef    mMaskBatch;

	gl::TextureRef	mTexture;
	gl::GlslProgRef	mShader;
	CameraPersp     mCamera;
	mat4            mTextureMatrix;

	Color mBlack = Color::black();
	Color mBlue = Color8u(66, 161, 235);
	Color mDarkBlue = Color8u::hex(0x1A3E5A);
	Color mRed = Color8u(240, 0, 0);
	// parameters
	ParameterBagRef				mParameterBag;
	// audio
	AudioWrapperRef				mAudio;
	// utils
	BatchassRef					mBatchass;
	// console
	AppConsoleRef				mConsole;

	bool						showConsole;
	void						ShowAppConsole(bool* opened);
	void						shift_left( std::size_t offset, std::size_t X);
	std::string					str;
};
