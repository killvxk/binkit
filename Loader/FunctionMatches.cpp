#include "FunctionMatches.h"
#include "DiffAlgorithms.h"

FunctionMatches::FunctionMatches(Binary& sourceBinary, Binary& targetBinary)
{
	m_sourceBinary = sourceBinary;
	m_targetBinary = targetBinary;
	m_psourceFunctions = sourceBinary.GetFunctions();
	m_ptargetFunctions = targetBinary.GetFunctions();
}

void FunctionMatches::Add(va_t sourceFunctionAddress, va_t targetFunctionAddress, MatchData matchData)
{
	unordered_map<va_t, TargetToMatchDataListMap>::iterator it = m_functionMatches.find(sourceFunctionAddress);
	if (it == m_functionMatches.end())
	{
		TargetToMatchDataListMap targetToMatchDataList;
		vector<MatchData> matchDataList;
		matchDataList.push_back(matchData);
		targetToMatchDataList.insert(pair<va_t, vector<MatchData>>(targetFunctionAddress, matchDataList));
		m_functionMatches.insert(pair<va_t, TargetToMatchDataListMap>(sourceFunctionAddress, targetToMatchDataList));
	}
	else
	{
		TargetToMatchDataListMap::iterator targetToMatchDataListMapit = it->second.find(targetFunctionAddress);

		if (targetToMatchDataListMapit == it->second.end())
		{
			vector<MatchData> matchDataList;
			matchDataList.push_back(matchData);
			it->second.insert(pair<va_t, vector<MatchData>>(targetFunctionAddress, matchDataList));
		}
		else
		{
			targetToMatchDataListMapit->second.push_back(matchData);
		}
	}
}

void FunctionMatches::Add(va_t sourceFunctionAddress, va_t targetFunctionAddress, vector<MatchData> matchDataList)
{
	for (MatchData matchData : matchDataList)
	{
		Add(sourceFunctionAddress, targetFunctionAddress, matchData);
	}
}

void FunctionMatches::Print()
{
	for (auto& val : m_functionMatches)
	{
		va_t sourceFunctionAddress = val.first;
		for (auto& val2 : val.second)
		{
			va_t targetFunctionAddress = val2.first;

			printf("==========================================\n");
			printf("Function: %x - %x\n", sourceFunctionAddress, targetFunctionAddress);
			for (MatchData matchData : val2.second)
			{
				printf("\tMatch: %x-%x %d\n", matchData.Source, matchData.Target, matchData.MatchRate);
			}
		}
	}
}

vector<FunctionMatch> FunctionMatches::GetMatches()
{
	vector<FunctionMatch> functionMatchDataList;

	for (auto& val : m_functionMatches)
	{
		va_t sourceFunctionAddress = val.first;
		for (auto& val2 : val.second)
		{
			FunctionMatch functionMatch;
			functionMatch.SourceFunction = sourceFunctionAddress;
			functionMatch.TargetFunction = val2.first;
			functionMatch.MatchDataList = val2.second;
			functionMatchDataList.push_back(functionMatch);
		}
	}

	return functionMatchDataList;
}

void FunctionMatches::AddMatches(vector<MatchData> currentMatchDataList)
{
	if (!m_psourceFunctions || !m_ptargetFunctions)
	{
		return;
	}

	for (MatchData matchData : currentMatchDataList)
	{
		va_t sourceFunctionAddress;
		m_psourceFunctions->GetFunctionAddress(matchData.Source, sourceFunctionAddress);
		va_t targetFunctionAddress;
		m_ptargetFunctions->GetFunctionAddress(matchData.Target, targetFunctionAddress);

		Add(sourceFunctionAddress, targetFunctionAddress, matchData);
	}
}

void FunctionMatches::DoInstructionHashMatch()
{
	DiffAlgorithms* p_diffAlgorithms = new DiffAlgorithms(m_sourceBinary, m_targetBinary);

	for (auto& val : m_functionMatches)
	{
		va_t sourceFunctionAddress = val.first;
		vector<va_t> sourceFunctionAddresses = m_psourceFunctions->GetBasicBlocks(sourceFunctionAddress);
		for (auto& val2 : val.second)
		{
			va_t targetFunctionAddress = val2.first;

			vector<va_t> targetFunctionAddresses = m_ptargetFunctions->GetBasicBlocks(targetFunctionAddress);
			vector<MatchData> functionInstructionHashMatches = p_diffAlgorithms->DoInstructionHashMatchInBlocks(sourceFunctionAddresses, targetFunctionAddresses);
			Add(sourceFunctionAddress, targetFunctionAddress, functionInstructionHashMatches);
		}
	}
}

void FunctionMatches::DoControlFlowMatch()
{
	DiffAlgorithms* p_diffAlgorithms = new DiffAlgorithms(m_sourceBinary, m_targetBinary);

	for (auto& val : m_functionMatches)
	{
		va_t sourceFunctionAddress = val.first;
		vector<va_t> sourceFunctionAddresses = m_psourceFunctions->GetBasicBlocks(sourceFunctionAddress);
		for (auto& val2 : val.second)
		{
			va_t targetFunctionAddress = val2.first;
			for (MatchData matchData : val2.second)
			{
				vector<MatchData> functionControlFlowMatches = p_diffAlgorithms->DoControlFlowMatch(matchData.Source, matchData.Target, CREF_FROM);
				Add(sourceFunctionAddress, targetFunctionAddress, functionControlFlowMatches);
			}
		}
	}
}
