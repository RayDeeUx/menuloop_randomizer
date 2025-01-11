#pragma once

class PlaylistModeWarning : public FLAlertLayer {
protected:
	bool init(bool isCringe);

public:
	static PlaylistModeWarning* create(bool isCringe);
};