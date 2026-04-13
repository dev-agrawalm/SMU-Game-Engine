#pragma once
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Game/Level.hpp"

class EditorOperation;
class LevelEditor;

//Class that represents a complete user command
class EditorCommand
{
public:
	explicit EditorCommand(LevelEditor* editor);
	~EditorCommand();
	void AddOperation(EditorOperation* operation);
	bool Execute();
	bool Undo();
	int GetOperationCount() const;
private:
	std::vector<EditorOperation*> m_operations;
	LevelEditor* m_editor = nullptr;
};
