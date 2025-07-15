#pragma once

class ConstantShuffleModeWarning : public FLAlertLayer {
protected:
	bool init(bool isCringe);

public:
	static ConstantShuffleModeWarning* create(bool isCringe);
};