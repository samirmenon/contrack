#include "ROIManager.h"
#include "DTIPathwayDatabase.h"
#include "DTIPathway.h"
#include "DTIQueryProcessor.h"
#include "ROIFactory.h"

using namespace std;

ROIManager::ROIManager()
{
	_nextId = 0;
	_selected.reset((DTIFilterROI*)0);
	_queryProc = new DTIQueryProcessor(this);
	_queryOp = DTI_FILTER_AND;
}
ROIManager::~ROIManager()
{
	delete _queryProc;
}
void ROIManager::erase(std::string name)
{
	PDTIFilterROIList::iterator index = find(name);
	
	//Selected has been deleted, so update the selected
	if(*index == _selected)
		if(size() == 1)
			_selected.reset((DTIFilterROI*)0);
		else
			_selected = *((PDTIFilterROIList*)(this))->begin();

	if(index != end())
		((PDTIFilterROIList*)(this))->erase(index);
	rebuildFilters();
}
void ROIManager::push_back(const PDTIFilterROI& elem)
{
	if(!_selected)
		_selected = elem;
	((PDTIFilterROIList*)(this))->push_back(elem);
	rebuildFilters();
}

void ROIManager::SetSelected(std::string name)
{
	PDTIFilterROIList::iterator index = find(name);
	if(index != end())
		_selected = *index;
}

DTIFilterROI *ROIManager::getROIFilter (const char *filterName)
{ 
	PDTIFilterROIList::iterator iter = find(filterName);
	if(iter == end())
		return 0;
	return (*iter).get();
}

void ROIManager::Filter(DTIPathwaySelection &in_selection, PDBHelper &helper, DTIFilterROI *voi)
{
	for(int i = 0; i < helper.PDB()->getNumFibers(); i++)
		if( in_selection[i] )
			in_selection[i] = voi->matches(helper.PDB()->getPathway(i));
}

void ROIManager::Filter(DTIPathwaySelection &in_selection, PDBHelper &helper)
{
	for(int i = 0; i < helper.PDB()->getNumFibers(); i++)
		if( in_selection[i] )
			in_selection[i] = _queryProc->doesPathwayMatch(helper.PDB()->getPathway(i));
}

void ROIManager::SetQueryOperation(DTIQueryOperation op) { _queryOp = op; rebuildFilters(); }


DTIFilterROI *ROIManager::Find(int id)
{
  PDTIFilterROIList::iterator iter = find(id);
  return iter == end() ? 0 : (*iter).get();
}

PDTIFilterROIList::iterator ROIManager::find(int id)
{
  PDTIFilterROIList::iterator index = end(), iter = ((PDTIFilterROIList*)(this))->begin();
	for(; iter != end(); iter++)
		if( (*iter)->ID() == id )
			return iter;
	return end();
}

DTIFilterROI *ROIManager::Find(std::string name)
{
	PDTIFilterROIList::iterator iter = find(name);
	return iter == end() ? 0 : (*iter).get();
}

PDTIFilterROIList::iterator ROIManager::find(std::string name)
{
  PDTIFilterROIList::iterator index = end(), iter = ((PDTIFilterROIList*)(this))->begin();
	for(; iter != end(); iter++)
		if( (*iter)->Name() == name )
			return iter;
	return end();
}

void ROIManager::rebuildFilters()
{
	char str[1024];
	string strFilter;
	for(PDTIFilterROIList::iterator iter=begin(); iter!=end() ; iter++) 
	{
		DTIFilterROI *current = (*iter).get();
		if (iter != begin()) 
			strFilter = strFilter + ( (_queryOp == DTI_FILTER_AND) ? " and " : " or ");

		sprintf (str, "%s", current->Name().c_str());
		strFilter.append(str);
	}
	_queryProc->setROIFilter (strFilter.c_str());
}

void ROIManager::Deserialize(ROIFactory *factory, std::istream &s, int version)
{
  clear(); // delete all existing ROIs
  Selected().reset();

  if (version < 2) {
    return; // version 1 files don't have ROIs
  }

  int n;

  s >>"--- ROIs ---" >> endl;
  s >>"Num ROIs: " >> n >> endl;
  for (int i = 0; i < n; i++){
    PDTIFilterROI pNew = factory->DeserializeROI(s, version);
    push_back(pNew);
  }
}

void ROIManager::Serialize (std::ostream &s, int version)
{
  s <<"--- ROIs ---" << endl;
  s <<"Num ROIs: " << size()<<endl;
  PDTIFilterROIList::iterator iter = ((PDTIFilterROIList*) this)->begin();
  for(; iter != end(); iter++) {
    PDTIFilterROI voi = (*iter);
    voi->Serialize(s, version);
  }
  s<<endl;
}
