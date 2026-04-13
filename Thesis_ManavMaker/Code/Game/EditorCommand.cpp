#include "Game/EditorCommand.hpp"
#include "Game/Tile.hpp"
#include "Game/LevelEditor.hpp"
#include "Game/EditorOperation.hpp"

EditorCommand::EditorCommand(LevelEditor* editor)
	: m_editor(editor)
{

}


EditorCommand::~EditorCommand()
{
	for (int i = 0; i < (int) m_operations.size(); i++)
	{
		EditorOperation*& op = m_operations[i];
		if (op)
		{
			delete op;
			op = nullptr;
		}
	}

	m_operations.clear();
}


void EditorCommand::AddOperation(EditorOperation* operation)
{
	if (operation == nullptr || operation->m_type == EDITOR_OPERATION_NONE)
		return;


	int duplicateCheckWindowSize = 5;
	int minmumIndex = GetMax((int) m_operations.size() - duplicateCheckWindowSize, 0);
	for (int index = (int) m_operations.size() - 1; index >= minmumIndex; index--)
	{
		EditorOperation*& op = m_operations[index];
		if (op->m_type == operation->m_type)
		{
			switch (op->m_type)
			{
				case EDITOR_OPERATION_TILE_PAINTING:
				{
					TilePaintingOperation* opA = dynamic_cast<TilePaintingOperation*>(op);
					TilePaintingOperation* opB = dynamic_cast<TilePaintingOperation*>(operation);
					if (opA->m_tileCoords == opB->m_tileCoords)
					{
						return;
					}
					break;
				}
				case EDITOR_OPERATION_TILE_ERASING:
				{
					TileErasingOperation* opA = dynamic_cast<TileErasingOperation*>(op);
					TileErasingOperation* opB = dynamic_cast<TileErasingOperation*>(operation);
					if (opA->m_tileCoords == opB->m_tileCoords)
					{
						return;
					}
					break;
				}
				case EDITOR_OPERATION_ENTITY_PAINTING:
				{
					EntityPaintingOperation* opA = dynamic_cast<EntityPaintingOperation*>(op);
					EntityPaintingOperation* opB = dynamic_cast<EntityPaintingOperation*>(operation);
					if (opA->m_spawnInfo == opB->m_spawnInfo)
					{
						return;
					}
					break;
				}
				case EDITOR_OPERATION_ENTITY_ERASING:
				{
					EntityErasingOperation* opA = dynamic_cast<EntityErasingOperation*>(op);
					EntityErasingOperation* opB = dynamic_cast<EntityErasingOperation*>(operation);
					if (opA->m_spawnInfo == opB->m_spawnInfo)
					{
						return;
					}
					break;
				}
				default:
					break;
			}
		}
	}

	m_operations.push_back(operation);
}


bool EditorCommand::Execute()
{
	for (int infoIndex = 0; infoIndex < m_operations.size(); infoIndex++)
	{
		EditorOperation* operation = m_operations[infoIndex];
		operation->Execute(m_editor);
	}
	return true;
}


bool EditorCommand::Undo()
{
	for (int operationIndex = 0; operationIndex < m_operations.size(); operationIndex++)
	{
		EditorOperation* operation = m_operations[operationIndex];
		operation->Undo(m_editor);
	}
	return true;
}


int EditorCommand::GetOperationCount() const
{
	return (int) m_operations.size();
}
