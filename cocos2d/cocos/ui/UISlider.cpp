/****************************************************************************
Copyright (c) 2013-2014 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "ui/UISlider.h"
#include "ui/UIScale9Sprite.h"
#include "ui/UIHelper.h"
#include "2d/CCSprite.h"

NS_CC_BEGIN

namespace ui {
    
static const int BASEBAR_RENDERER_Z = (-2);
static const int PROGRESSBAR_RENDERER_Z = (-2);
static const int SLIDBALL_RENDERER_Z = (-1);
    
IMPLEMENT_CLASS_GUI_INFO(Slider)
    
Slider::Slider():
_barRenderer(nullptr),
_progressBarRenderer(nullptr),
_barTextureSize(Size::ZERO),
_progressBarTextureSize(Size::ZERO),
_slidBallNormalRenderer(nullptr),
_slidBallPressedRenderer(nullptr),
_slidBallDisabledRenderer(nullptr),
_slidBallRenderer(nullptr),
_barLength(0.0),
_percent(0),
_scale9Enabled(false),
_prevIgnoreSize(true),
_zoomScale(0.1f),
_sliderBallNormalTextureScaleX(1.0),
_sliderBallNormalTextureScaleY(1.0),
_isSliderBallPressedTextureLoaded(false),
_isSliderBallDisabledTexturedLoaded(false),
_capInsetsBarRenderer(Rect::ZERO),
_capInsetsProgressBarRenderer(Rect::ZERO),
_sliderEventListener(nullptr),
_sliderEventSelector(nullptr),
_eventCallback(nullptr),
_barTexType(TextureResType::LOCAL),
_progressBarTexType(TextureResType::LOCAL),
_ballNTexType(TextureResType::LOCAL),
_ballPTexType(TextureResType::LOCAL),
_ballDTexType(TextureResType::LOCAL),
_barRendererAdaptDirty(true),
_progressBarRendererDirty(true)
{
    setTouchEnabled(true);
}

Slider::~Slider()
{
    _sliderEventListener = nullptr;
    _sliderEventSelector = nullptr;
}

Slider* Slider::create()
{
    Slider* widget = new (std::nothrow) Slider();
    if (widget && widget->init())
    {
        widget->autorelease();
        return widget;
    }
    CC_SAFE_DELETE(widget);
    return nullptr;
}
    
Slider* Slider::create(const std::string& barTextureName,
                      const std::string& normalBallTextureName,
                      TextureResType resType)
{
    Slider* widget = new (std::nothrow) Slider();
    if (widget && widget->init())
    {
        widget->loadBarTexture(barTextureName, resType);
        widget->loadSlidBallTextureNormal(normalBallTextureName, resType);
        widget->autorelease();
        return widget;
    }
    CC_SAFE_DELETE(widget);
    return nullptr;
}

bool Slider::init()
{
    if (Widget::init())
    {
        return true;
    }
    return false;
}

void Slider::initRenderer()
{
    _barRenderer = Scale9Sprite::create();
    _progressBarRenderer = Scale9Sprite::create();
    _barRenderer->setScale9Enabled(false);
    _progressBarRenderer->setScale9Enabled(false);
    
    _progressBarRenderer->setAnchorPoint(Vec2(0.0f, 0.5f));
    
    addProtectedChild(_barRenderer, BASEBAR_RENDERER_Z, -1);
    addProtectedChild(_progressBarRenderer, PROGRESSBAR_RENDERER_Z, -1);
    
    _slidBallNormalRenderer = Sprite::create();
    _slidBallPressedRenderer = Sprite::create();
    _slidBallPressedRenderer->setVisible(false);
    _slidBallDisabledRenderer = Sprite::create();
    _slidBallDisabledRenderer->setVisible(false);
    
    _slidBallRenderer = Node::create();
    
    _slidBallRenderer->addChild(_slidBallNormalRenderer);
    _slidBallRenderer->addChild(_slidBallPressedRenderer);
    _slidBallRenderer->addChild(_slidBallDisabledRenderer);
    _slidBallRenderer->setCascadeColorEnabled(true);
    _slidBallRenderer->setCascadeOpacityEnabled(true);
    
    addProtectedChild(_slidBallRenderer, SLIDBALL_RENDERER_Z, -1);
}

void Slider::loadBarTexture(const std::string& fileName, TextureResType texType)
{
    if (fileName.empty())
    {
        return;
    }
    _barTexType = texType;
    switch (_barTexType)
    {
        case TextureResType::LOCAL:
            _barRenderer->initWithFile(fileName);
            break;
        case TextureResType::PLIST:
            _barRenderer->initWithSpriteFrameName(fileName);
            break;
        default:
            break;
    }
    this->setupBarTexture();
}
void Slider::loadBarTexture(SpriteFrame* spriteframe)
{
    _barRenderer->initWithSpriteFrame(spriteframe);
    this->setupBarTexture();
}

void Slider::setupBarTexture()
{
    this->updateChildrenDisplayedRGBA();
    _barRendererAdaptDirty = true;
    _progressBarRendererDirty = true;
    updateContentSizeWithTextureSize(_barRenderer->getContentSize());
    _barTextureSize = _barRenderer->getContentSize();
}

void Slider::loadProgressBarTexture(const std::string& fileName, TextureResType texType)
{
    if (fileName.empty())
    {
        return;
    }
    _progressBarTexType = texType;
    switch (_progressBarTexType)
    {
        case TextureResType::LOCAL:
            _progressBarRenderer->initWithFile(fileName);
            break;
        case TextureResType::PLIST:
            _progressBarRenderer->initWithSpriteFrameName(fileName);
            break;
        default:
            break;
    }
    this->setupProgressBarTexture();
}

void Slider::loadProgressBarTexture(SpriteFrame* spriteframe)
{
    _progressBarRenderer->initWithSpriteFrame(spriteframe);
    this->setupProgressBarTexture();
}

void Slider::setupProgressBarTexture()
{
    this->updateChildrenDisplayedRGBA();
    _progressBarRenderer->setAnchorPoint(Vec2(0.0f, 0.5f));
    _progressBarTextureSize = _progressBarRenderer->getContentSize();
    _progressBarRendererDirty = true;
}

void Slider::setScale9Enabled(bool able)
{
    if (_scale9Enabled == able)
    {
        return;
    }
    
    _scale9Enabled = able;
    _barRenderer->setScale9Enabled(_scale9Enabled);
    _progressBarRenderer->setScale9Enabled(_scale9Enabled);
    
    if (_scale9Enabled)
    {
        bool ignoreBefore = _ignoreSize;
        ignoreContentAdaptWithSize(false);
        _prevIgnoreSize = ignoreBefore;
    }
    else
    {
        ignoreContentAdaptWithSize(_prevIgnoreSize);
    }
    setCapInsetsBarRenderer(_capInsetsBarRenderer);
    setCapInsetProgressBarRebderer(_capInsetsProgressBarRenderer);
    _barRendererAdaptDirty = true;
    _progressBarRendererDirty = true;
}
    
bool Slider::isScale9Enabled()const
{
    return _scale9Enabled;
}

void Slider::ignoreContentAdaptWithSize(bool ignore)
{
    if (!_scale9Enabled || (_scale9Enabled && !ignore))
    {
        Widget::ignoreContentAdaptWithSize(ignore);
        _prevIgnoreSize = ignore;
    }
}

void Slider::setCapInsets(const Rect &capInsets)
{
    setCapInsetsBarRenderer(capInsets);
    setCapInsetProgressBarRebderer(capInsets);
}

void Slider::setCapInsetsBarRenderer(const Rect &capInsets)
{
    _capInsetsBarRenderer = ui::Helper::restrictCapInsetRect(capInsets, _barRenderer->getContentSize());
    if (!_scale9Enabled)
    {
        return;
    }
    _barRenderer->setCapInsets(_capInsetsBarRenderer);
}
    
const Rect& Slider::getCapInsetsBarRenderer()const
{
    return _capInsetsBarRenderer;
}

void Slider::setCapInsetProgressBarRebderer(const Rect &capInsets)
{
    _capInsetsProgressBarRenderer = ui::Helper::restrictCapInsetRect(capInsets, _progressBarRenderer->getContentSize());
    
    if (!_scale9Enabled)
    {
        return;
    }
    _progressBarRenderer->setCapInsets(_capInsetsProgressBarRenderer);
}
    
const Rect& Slider::getCapInsetsProgressBarRebderer()const
{
    return _capInsetsProgressBarRenderer;
}

void Slider::loadSlidBallTextures(const std::string& normal,
                                      const std::string& pressed,
                                      const std::string& disabled,
                                      TextureResType texType)
{
    loadSlidBallTextureNormal(normal, texType);
    loadSlidBallTexturePressed(pressed,texType);
    loadSlidBallTextureDisabled(disabled,texType);
}

void Slider::loadSlidBallTextureNormal(const std::string& normal,TextureResType texType)
{
    if (normal.empty())
    {
        return;
    }
    _ballNTexType = texType;
    switch (_ballNTexType)
    {
        case TextureResType::LOCAL:
            _slidBallNormalRenderer->setTexture(normal);
            break;
        case TextureResType::PLIST:
            _slidBallNormalRenderer->setSpriteFrame(normal);
            break;
        default:
            break;
    }
    this->updateChildrenDisplayedRGBA();
}
void Slider::loadSlidBallTextureNormal(SpriteFrame* spriteframe)
{
    _slidBallNormalRenderer->setSpriteFrame(spriteframe);
    this->updateChildrenDisplayedRGBA();
}

void Slider::loadSlidBallTexturePressed(const std::string& pressed,TextureResType texType)
{
    if (pressed.empty())
    {
        return;
    }
    _ballPTexType = texType;
    _isSliderBallPressedTextureLoaded = true;
    switch (_ballPTexType)
    {
        case TextureResType::LOCAL:
            _slidBallPressedRenderer->setTexture(pressed);
            break;
        case TextureResType::PLIST:
            _slidBallPressedRenderer->setSpriteFrame(pressed);
            break;
        default:
            break;
    }
    this->updateChildrenDisplayedRGBA();
}


void Slider::loadSlidBallTexturePressed(SpriteFrame* spriteframe)
{
    _slidBallPressedRenderer->setSpriteFrame(spriteframe);
    this->updateChildrenDisplayedRGBA();
}

void Slider::loadSlidBallTextureDisabled(const std::string& disabled,TextureResType texType)
{
    if (disabled.empty())
    {
        return;
    }
    _isSliderBallDisabledTexturedLoaded = true;
    _ballDTexType = texType;
    switch (_ballDTexType)
    {
        case TextureResType::LOCAL:
            _slidBallDisabledRenderer->setTexture(disabled);
            break;
        case TextureResType::PLIST:
            _slidBallDisabledRenderer->setSpriteFrame(disabled);
            break;
        default:
            break;
    }
    this->updateChildrenDisplayedRGBA();
}

void Slider::loadSlidBallTextureDisabled(SpriteFrame* spriteframe)
{
    _slidBallDisabledRenderer->setSpriteFrame(spriteframe);
    this->updateChildrenDisplayedRGBA();
}

void Slider::setPercent(int percent)
{
    if (percent > 100)
    {
        percent = 100;
    }
    if (percent < 0)
    {
        percent = 0;
    }
    _percent = percent;
    float res = percent / 100.0f;
    float dis = _barLength * res;
    _slidBallRenderer->setPosition(dis, _contentSize.height / 2.0f);
    if (_scale9Enabled)
    {
        _progressBarRenderer->setPreferredSize(Size(dis,_contentSize.height));
    }
    else
    {
        Sprite* spriteRenderer = _progressBarRenderer->getSprite();
        
        if (nullptr != spriteRenderer) {
            Rect rect = spriteRenderer->getTextureRect();
            rect.size.width = _progressBarTextureSize.width * res;
            spriteRenderer->setTextureRect(rect, spriteRenderer->isTextureRectRotated(), rect.size);
        }
    }
}
    
bool Slider::hitTest(const cocos2d::Vec2 &pt)
{
    Vec2 nsp = this->_slidBallNormalRenderer->convertToNodeSpace(pt);
    Size ballSize = this->_slidBallNormalRenderer->getContentSize();
    Rect ballRect = Rect(0,0, ballSize.width, ballSize.height);
    if (ballRect.containsPoint(nsp))
    {
        return true;
    }
    return false;
}

bool Slider::onTouchBegan(Touch *touch, Event *unusedEvent)
{
    bool pass = Widget::onTouchBegan(touch, unusedEvent);
    if (_hitted)
    {
        Vec2 nsp = convertToNodeSpace(_touchBeganPosition);
        setPercent(getPercentWithBallPos(nsp.x));
        percentChangedEvent();
    }
    return pass;
}

void Slider::onTouchMoved(Touch *touch, Event *unusedEvent)
{
    _touchMovePosition = touch->getLocation();
    Vec2 nsp = convertToNodeSpace(_touchMovePosition);
    setPercent(getPercentWithBallPos(nsp.x));
    percentChangedEvent();
}

void Slider::onTouchEnded(Touch *touch, Event *unusedEvent)
{
    Widget::onTouchEnded(touch, unusedEvent);
}

void Slider::onTouchCancelled(Touch *touch, Event *unusedEvent)
{
    Widget::onTouchCancelled(touch, unusedEvent);
}

float Slider::getPercentWithBallPos(float px)const
{
    return ((px/_barLength)*100.0f);
}

void Slider::addEventListenerSlider(Ref *target, SEL_SlidPercentChangedEvent selector)
{
    _sliderEventListener = target;
    _sliderEventSelector = selector;
}
    
void Slider::addEventListener(const ccSliderCallback& callback)
{
    _eventCallback = callback;
}

void Slider::percentChangedEvent()
{
    this->retain();
    if (_sliderEventListener && _sliderEventSelector)
    {
        (_sliderEventListener->*_sliderEventSelector)(this,SLIDER_PERCENTCHANGED);
    }
    if (_eventCallback)
    {
        _eventCallback(this, EventType::ON_PERCENTAGE_CHANGED);
    }
    if (_ccEventCallback)
    {
        _ccEventCallback(this, static_cast<int>(EventType::ON_PERCENTAGE_CHANGED));
    }
    this->release();
}

int Slider::getPercent()const
{
    return _percent;
}

void Slider::onSizeChanged()
{
    Widget::onSizeChanged();
    _barRendererAdaptDirty = true;
    _progressBarRendererDirty = true;
}
    
void Slider::adaptRenderers()
{
    if (_barRendererAdaptDirty)
    {
        barRendererScaleChangedWithSize();
        _barRendererAdaptDirty = false;
    }
    if (_progressBarRendererDirty)
    {
        progressBarRendererScaleChangedWithSize();
        _progressBarRendererDirty = false;
    }
}

Size Slider::getVirtualRendererSize() const
{
    return _barRenderer->getContentSize();
}

Node* Slider::getVirtualRenderer()
{
    return _barRenderer;
}

void Slider::barRendererScaleChangedWithSize()
{
    if (_unifySize)
    {
        _barLength = _contentSize.width;
        _barRenderer->setPreferredSize(_contentSize);
    }
    else if (_ignoreSize)
    {
        
        _barRenderer->setScale(1.0f);
        _barLength = _contentSize.width;
    }
    else
    {
        _barLength = _contentSize.width;
        if (_scale9Enabled)
        {
            _barRenderer->setPreferredSize(_contentSize);
            _barRenderer->setScale(1.0f);
        }
        else
        {
            Size btextureSize = _barTextureSize;
            if (btextureSize.width <= 0.0f || btextureSize.height <= 0.0f)
            {
                _barRenderer->setScale(1.0f);
            }
            else
            {
                float bscaleX = _contentSize.width / btextureSize.width;
                float bscaleY = _contentSize.height / btextureSize.height;
                _barRenderer->setScaleX(bscaleX);
                _barRenderer->setScaleY(bscaleY);
            }
        }
    }
    _barRenderer->setPosition(_contentSize.width / 2.0f, _contentSize.height / 2.0f);
    setPercent(_percent);
}

void Slider::progressBarRendererScaleChangedWithSize()
{
    if (_unifySize)
    {
        _progressBarRenderer->setPreferredSize(_contentSize);
    }
    else if (_ignoreSize)
    {
        if (!_scale9Enabled)
        {
            Size ptextureSize = _progressBarTextureSize;
            float pscaleX = _contentSize.width / ptextureSize.width;
            float pscaleY = _contentSize.height / ptextureSize.height;
            _progressBarRenderer->setScaleX(pscaleX);
            _progressBarRenderer->setScaleY(pscaleY);
        }
    }
    else
    {
        if (_scale9Enabled)
        {
            _progressBarRenderer->setPreferredSize(_contentSize);
            _progressBarRenderer->setScale(1.0);
        }
        else
        {
            Size ptextureSize = _progressBarTextureSize;
            if (ptextureSize.width <= 0.0f || ptextureSize.height <= 0.0f)
            {
                _progressBarRenderer->setScale(1.0f);
                return;
            }
            float pscaleX = _contentSize.width / ptextureSize.width;
            float pscaleY = _contentSize.height / ptextureSize.height;
            _progressBarRenderer->setScaleX(pscaleX);
            _progressBarRenderer->setScaleY(pscaleY);
        }
    }
    _progressBarRenderer->setPosition(0.0f, _contentSize.height / 2.0f);
    setPercent(_percent);
}

void Slider::onPressStateChangedToNormal()
{
    _slidBallNormalRenderer->setVisible(true);
    _slidBallPressedRenderer->setVisible(false);
    _slidBallDisabledRenderer->setVisible(false);
    
    _slidBallNormalRenderer->setGLProgramState(this->getNormalGLProgramState());
    _slidBallNormalRenderer->setScale(_sliderBallNormalTextureScaleX, _sliderBallNormalTextureScaleY);
}

void Slider::onPressStateChangedToPressed()
{
    _slidBallNormalRenderer->setGLProgramState(this->getNormalGLProgramState());

    
    if (!_isSliderBallPressedTextureLoaded)
    {
        _slidBallNormalRenderer->setScale(_sliderBallNormalTextureScaleX + _zoomScale,
                                          _sliderBallNormalTextureScaleY + _zoomScale);
    }
    else
    {
        _slidBallNormalRenderer->setVisible(false);
        _slidBallPressedRenderer->setVisible(true);
        _slidBallDisabledRenderer->setVisible(false);
    }
}

void Slider::onPressStateChangedToDisabled()
{
    if (!_isSliderBallDisabledTexturedLoaded)
    {
        _slidBallNormalRenderer->setGLProgramState(this->getGrayGLProgramState());
        _slidBallNormalRenderer->setVisible(true);
    }
    else
    {
        _slidBallNormalRenderer->setVisible(false);
        _slidBallDisabledRenderer->setVisible(true);
    }
    
    _slidBallNormalRenderer->setScale(_sliderBallNormalTextureScaleX, _sliderBallNormalTextureScaleY);
    
    _slidBallPressedRenderer->setVisible(false);
}
    
    
void Slider::setZoomScale(float scale)
{
    _zoomScale = scale;
}

float Slider::getZoomScale()const
{
    return _zoomScale;
}


std::string Slider::getDescription() const
{
    return "Slider";
}

Widget* Slider::createCloneInstance()
{
    return Slider::create();
}

void Slider::copySpecialProperties(Widget *widget)
{
    Slider* slider = dynamic_cast<Slider*>(widget);
    if (slider)
    {
        _prevIgnoreSize = slider->_prevIgnoreSize;
        setScale9Enabled(slider->_scale9Enabled);
        auto barSprite = slider->_barRenderer->getSprite();
        if (nullptr != barSprite)
        {
            loadBarTexture(barSprite->getSpriteFrame());
        }
        auto progressSprite = slider->_progressBarRenderer->getSprite();
        if (nullptr != progressSprite)
        {
            loadProgressBarTexture(progressSprite->getSpriteFrame());
        }
        loadSlidBallTextureNormal(slider->_slidBallNormalRenderer->getSpriteFrame());
        loadSlidBallTexturePressed(slider->_slidBallPressedRenderer->getSpriteFrame());
        loadSlidBallTextureDisabled(slider->_slidBallDisabledRenderer->getSpriteFrame());
        setPercent(slider->getPercent());
        _isSliderBallPressedTextureLoaded = slider->_isSliderBallPressedTextureLoaded;
        _isSliderBallDisabledTexturedLoaded = slider->_isSliderBallDisabledTexturedLoaded;
        _sliderEventListener = slider->_sliderEventListener;
        _sliderEventSelector = slider->_sliderEventSelector;
        _eventCallback = slider->_eventCallback;
        _ccEventCallback = slider->_ccEventCallback;
    }
}

}

NS_CC_END
