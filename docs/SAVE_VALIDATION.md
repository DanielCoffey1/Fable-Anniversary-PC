# Save persistence investigation

The user reported losing Oakvale deed progress after restarting, despite using a manual save. Existing profile, autosave, quick-autosave, manual-slot, and quest-checkpoint files were found under the runtime's default user-data folder (`Documents/fable_anniversary`). They were backed up locally under ignored `work/save-backup-*` before testing.

A fresh manual save at 12:18:30 EDT on October 16, 2026 changed the manual-slot file's timestamp and SHA-256. Its 307,200-byte payload exactly matched the existing quest checkpoint (SHA-256 `D3C638A7F9989DE6CB13152CFF238AA30714031AB974AC5A79F2B2F72C742542`). The supplied screenshot explicitly labels the operation **Save Last Checkpoint**. Thus the tested write reaches disk; this does not establish that every later checkpoint advances correctly.

[The Anniversary Steam save-system discussion](https://steamcommunity.com/app/288470/discussions/0/613937942849287182/) describes autosaves at quest boundaries, checkpoints at region transitions, and manual saves during a quest copying the latest checkpoint. This is consistent with the observed file comparison. Individual deeds within the Oakvale birthday sequence should not be assumed to be separate save boundaries.

The next test is to finish the birthday sequence and reach the Heroes' Guild, check for new autosave/checkpoint payloads, then verify a reload. No save logic or source content has been changed. Startup warnings about `GAME:\Saves` and `g:\AutoSave.rl` remain separate observations; they have not been demonstrated to prevent the Xbox content-package saves that were written successfully.
