//  Created by Andrew Morton https://github.com/drewish/AlienLander

#include "UnionJackApp.h"

void prepareSettings(App::Settings *settings)
{
	settings->setMultiTouchEnabled(true);
	settings->setHighDensityDisplayEnabled();
	settings->setWindowSize(800, 800);
}
// -------- SPOUT -------------
void UnionJackApp::setup()
{
	try {
		mTexture = gl::Texture::create(loadImage(loadResource(RES_US_SQUARE)));
		mTexture->bind(0);
	}
	catch (...) {
		console() << "unable to load the texture file!" << std::endl;
	}
	mTexture->setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

	try {
		mShader = ci::gl::GlslProg::create(
			ci::app::loadResource(RES_VERT),
			ci::app::loadResource(RES_FRAG)
			);
	}
	catch (gl::GlslProgCompileExc &exc) {
		console() << "Shader compile error: " << std::endl;
		console() << exc.what();
	}
	catch (...) {
		console() << "Unable to load shader" << std::endl;
	}


	buildMeshes();

	mShip.setup();

	mDisplays.push_back(SegmentDisplay(10).position(vec2(5)).scale(2));
	mDisplays.push_back(SegmentDisplay(10).rightOf(mDisplays.back()));
	mDisplays.push_back(SegmentDisplay(35).below(mDisplays.front()));
	mDisplays.push_back(SegmentDisplay(35).below(mDisplays.back()));

	for (auto display = mDisplays.begin(); display != mDisplays.end(); ++display) {
		display->colors(ColorA(mBlue, 0.8), ColorA(mDarkBlue, 0.4));
		display->setup();
	}

	//    setFullScreen( true );
	setFrameRate(60);
	gl::enableVerticalSync(true);
}
void UnionJackApp::buildMeshes()
{
	vector<vec3> lineCoords;
	vector<vec3> maskCoords;

	for (unsigned int z = 0; z < mLines; ++z) {
		for (unsigned int x = 0; x < mPoints; ++x) {
			vec3 vert = vec3(x / (float)mPoints, 1, z / (float)mLines);

			lineCoords.push_back(vert);

			// To speed up the vertex shader it only does the texture lookup
			// for vertexes with y values greater than 0. This way we can build
			// a strip: 1 1 1  that will become: 2 9 3
			//          |\|\|                    |\|\|
			//          0 0 0                    0 0 0
			maskCoords.push_back(vert);
			vert.y = 0.0;
			maskCoords.push_back(vert);
		}
	}
	gl::VboMeshRef lineMesh = gl::VboMesh::create(lineCoords.size(), GL_LINE_STRIP, {
		gl::VboMesh::Layout().usage(GL_STATIC_DRAW).attrib(geom::Attrib::POSITION, 3),
	});
	lineMesh->bufferAttrib(geom::Attrib::POSITION, lineCoords);
	mLineBatch = gl::Batch::create(lineMesh, mShader);

	gl::VboMeshRef maskMesh = gl::VboMesh::create(maskCoords.size(), GL_TRIANGLE_STRIP, {
		gl::VboMesh::Layout().usage(GL_STATIC_DRAW).attrib(geom::Attrib::POSITION, 3),
	});
	maskMesh->bufferAttrib(geom::Attrib::POSITION, maskCoords);
	mMaskBatch = gl::Batch::create(maskMesh, mShader);
}
void UnionJackApp::mouseMove(MouseEvent event)
{
	//    int height = getWindowHeight();
	//    int width = getWindowWidth();
	//    mZoom = 1 - (math<float>::clamp(event.getY(), 0, height) / height);
	//    mAngle = (math<float>::clamp(event.getX(), 0, width) / width);
	//    2 * M_PI *
}

void UnionJackApp::touchesMoved(TouchEvent event)
{
	vec2 mDelta1, mDelta2;

	// TODO treat the two deltas as forces acting on a rigid body.
	// Acceleration becomes translation
	// Torque becomes rotation
	// Compression/tension becomes zooming
	const vector<TouchEvent::Touch>&touches = event.getTouches();
	if (touches.size() == 2) {
		mDelta1 = touches[0].getPrevPos() - touches[0].getPos();
		mDelta2 = touches[1].getPrevPos() - touches[1].getPos();

		mShip.mPos.x += (mDelta1.x + mDelta2.x) / 768.0;
		mShip.mPos.y += (mDelta1.y + mDelta2.y) / 768.0;
	}
}

void UnionJackApp::keyDown(KeyEvent event)
{
	switch (event.getCode()) {
	case KeyEvent::KEY_ESCAPE:
		quit();
		break;
	case KeyEvent::KEY_h:
		mShowHud = !mShowHud;
		break;
	case KeyEvent::KEY_c:
		mShowCompass = !mShowCompass;
		break;
	default:
		mShip.keyDown(event);
		break;
	}
}

void UnionJackApp::keyUp(KeyEvent event)
{
	mShip.keyUp(event);
}

void UnionJackApp::resize()
{
	unsigned int height = getWindowHeight();
	unsigned int width = getWindowWidth();
	unsigned int margin = 20;
	mPoints = (width - (2 * margin)) / 10.0;
	mLines = (height - (2 * margin)) / 25.0;
	buildMeshes();
}
void UnionJackApp::update()
{
	mShip.update();

	// TODO Need to figure our what to do with the scale... should probably
	// affect the distance to the points rather than being handled by moving
	// the camera...
	float scale = math<float>::clamp(mShip.mPos.z, 0.2, 1.0);
	mTextureMatrix = glm::translate(vec3(0.5, 0.5, 0));
	mTextureMatrix = glm::rotate(mTextureMatrix, mShip.mPos.w, vec3(0, 0, 1));
	mTextureMatrix = glm::scale(mTextureMatrix, vec3(scale, scale, 0.25));
	mTextureMatrix = glm::translate(mTextureMatrix, vec3(mShip.mPos.x, mShip.mPos.y, 0));
	mTextureMatrix = glm::translate(mTextureMatrix, vec3(-0.5, -0.5, 0));

	// TODO: Need to change the focus point to remain parallel as we descend
	mCamera.setPerspective(40.0f, 1.0f, 0.5f, 3.0f);
	mCamera.lookAt(vec3(0.0f, 1.5f * mShip.mPos.z, 1.0f), vec3(0.0, 0.1, 0.0), vec3(0, 1, 0));

	if (mShowHud) {
		const vec4 &vel = mShip.mVel;
		const vec4 &acc = mShip.mAcc;
		float fps = getAverageFps();
		boost::format zeroToOne("%+07.5f");
		boost::format shortForm("%+08.4f");

		mDisplays[0].display("ALT " + (zeroToOne % mShip.mPos.z).str());
		mDisplays[1]
			.display("FPS " + (shortForm % fps).str())
			.colors(ColorA(fps < 50 ? mRed : mBlue, 0.8), ColorA(mDarkBlue, 0.8));
		mDisplays[2].display(
			" X " + (shortForm % vel.x).str() + " " +
			" Y " + (shortForm % vel.y).str() + " " +
			" R " + (shortForm % vel.w).str()
			);
		mDisplays[3].display(
			"dX " + (shortForm % acc.x).str() + " " +
			"dY " + (shortForm % acc.y).str() + " " +
			"dR " + (shortForm % acc.w).str()
			);
	}
}

void UnionJackApp::cleanup()
{
    
}

void UnionJackApp::mouseDown(MouseEvent event)
{
  
}
// ----------------------------


void UnionJackApp::draw()
{
	gl::clear(mBlack, true);

	{
		gl::ScopedMatrices matrixScope;
		gl::setMatrices(mCamera);

		gl::ScopedDepth depthScope(true);

		mShader->uniform("textureMatrix", mTextureMatrix);

		// Center the model
		gl::translate(-0.5, 0.0, -0.5);

		unsigned int indiciesInLine = mPoints;
		unsigned int indiciesInMask = mPoints * 2;
		// Draw front to back to take advantage of the depth buffer.
		for (int i = mLines - 1; i >= 0; --i) {
			gl::color(mBlack);
			// Draw masks with alternating colors for debugging
			// gl::color( Color::gray( i % 2 == 1 ? 0.5 : 0.25) );
			mMaskBatch->draw(i * indiciesInMask, indiciesInMask);

			gl::color(mBlue);
			mLineBatch->draw(i * indiciesInLine, indiciesInLine);
		}
	}

	// Compass vector pointing north
	if (mShowCompass) {
		gl::color(mRed);
		vec3 origin = vec3(0.5, 0.2, 0.5);
		vec3 heading = glm::rotateY(vec3(0.01, 0, 0), mShip.mPos.w);
		gl::drawVector(origin, origin + heading, 0.05, 0.01);
	}

	if (mShowHud) {
		for (auto display = mDisplays.begin(); display != mDisplays.end(); ++display) {
			display->draw();
		}
	}
}


CINDER_APP(UnionJackApp, RendererGl(RendererGl::Options().msaa(16)), prepareSettings)
