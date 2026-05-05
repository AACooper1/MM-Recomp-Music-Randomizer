#include "songslot.h"
#include "seed.h"

void Seed::apply_songforce()
{
    for (const auto & [ id, track ] : randomized)
    {
        if (songforceTracks.contains(track->seedIdx))
        {
            songSlots[id].is_songforce = true;
            songforceTracks.erase(track->seedIdx);
        }
    }
    std::vector<int> randomOrder;
    for (int i = 2; i < songSlots.size(); i++)
    {
        randomOrder.push_back(i);
    }

    rng.seed(this->seed);
    std::ranges::shuffle(randomOrder, rng);

    for (int s = 0; s < randomOrder.size(); s++)
    {
        int slotIdx = randomOrder[s];
        for (int i = 0; i < songSlots[slotIdx].availableTracks.size(); i++)
        {
            int trackId = songSlots[slotIdx].availableTracks[i];
            if (songforceTracks.contains(trackId))
            {
                if (!tracks[trackId]->is_prepared)
                {
                    db->prepare_track(trackId);
                }
                randomized[slotIdx] = tracks[trackId];
                songSlots[slotIdx].is_songforce = true;
                songforceTracks.erase(trackId);

                clear_track_availability(trackId);

                logger.debug << "Songforce'd track " << trackId << " (" << tracks[trackId]->name << ") into slot " << slotIdx << " (" << songSlots[slotIdx].name << ")!" << std::endl;
            }
        }
    }
}

void Seed::apply_songtest()
{
    for (const auto & [ id, track ] : tracks)
    {
        if (track->path.filename().string().contains("songtest"))
        {
            logger.info << "SONGTEST:" << std::endl;
            logger.info.disable_header();
            randomized[(int)SongSlotID::FILE_SELECT] = track;
            for (int i = 0; i < songSlots.size(); i++)
            {
                if (std::find(songSlots[i].availableTracksNoRemove.begin(), songSlots[i].availableTracksNoRemove.end(), track->seedIdx) != songSlots[i].availableTracksNoRemove.end())
                {
                    randomized[i] = track;
                    logger.info << "\t " << songSlots[i].name << " --> " << track->name << std::endl;
                }
            }
            logger.info.enable_header();

            prepare_tracks();

            return;
        }
    }
}